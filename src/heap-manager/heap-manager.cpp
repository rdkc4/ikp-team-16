#include "heap-manager.hpp"

#include <latch>
#include <mutex>

heap_manager::heap_manager(size_t hm_thread_count, size_t gc_thread_count) : heap_manager_thread_pool(hm_thread_count), gc(gc_thread_count) {
    for(size_t i = 0; i < SMALL_OBJECT_SEGMENTS; ++i) {
        segment& segment = heap_memory.get_small_object_segment(i);
        header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
        free_memory_table.update_segment(i, initial_header, segment.free_memory);
    }

    for(size_t i = 0; i < MEDIUM_OBJECT_SEGMENTS; ++i) {
        segment& segment = heap_memory.get_medium_object_segment(i);
        header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
        free_memory_table.update_segment(SMALL_OBJECT_SEGMENTS + i, initial_header, segment.free_memory);
    }

    for(size_t i = 0; i < LARGE_OBJECT_SEGMENTS; ++i) {
        segment& segment = heap_memory.get_large_object_segment(i);
        header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
        free_memory_table.update_segment(SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + i, initial_header, segment.free_memory);
    }
}

header* heap_manager::allocate(uint32_t bytes){
    if(bytes == 0) return nullptr;
    bytes = (bytes + 15) & ~15;

    constexpr size_t FAST_ATTEMPTS = 3;

    for(size_t i = 0; i < FAST_ATTEMPTS; ++i){
        int segment_index = find_suitable_segment(bytes);
        if(segment_index >= 0){
            std::lock_guard<std::mutex> seg_lock(segment_locks[segment_index]);
            if(header* obj = allocate_from_segment(static_cast<size_t>(segment_index), bytes))
                return obj;
        }
    }
    
    bool expected = false;
    if(gc_in_progress.compare_exchange_strong(expected, true, std::memory_order_acq_rel)){
        collect_garbage();
        gc_in_progress.store(false, std::memory_order_release);
        gc_in_progress.notify_all();
    }
    else {
        while(gc_in_progress.load(std::memory_order_acquire)){
            gc_in_progress.wait(true);
        }
    }

    int segment_index = find_suitable_segment(bytes);
    if(segment_index >= 0){
        std::lock_guard<std::mutex> seg_lock(segment_locks[segment_index]);
        return allocate_from_segment(static_cast<size_t>(segment_index), bytes);
    }

    return nullptr;
}

void heap_manager::add_root(std::string key, std::unique_ptr<root_set_base> base){
    std::lock_guard<std::mutex> root_set_lock(root_set_mutex);
    root_set.add_root(std::move(key), std::move(base));
}

root_set_base* heap_manager::get_root(const std::string& key) {
    std::lock_guard<std::mutex> root_set_lock(root_set_mutex);
    return root_set.get_root(key);
}

void heap_manager::remove_root(const std::string& key){
    std::lock_guard<std::mutex> root_set_lock(root_set_mutex);
    root_set.remove_root(key);
}

void heap_manager::clear_roots() noexcept {
    std::lock_guard<std::mutex> root_set_lock(root_set_mutex);
    root_set.clear();
}

void heap_manager::collect_garbage(){
    std::lock_guard<std::mutex> root_set_lock(root_set_mutex);

    std::unique_lock<std::mutex> locks[TOTAL_SEGMENTS];
    for(size_t i = 0; i < TOTAL_SEGMENTS; ++i){
        locks[i] = std::unique_lock<std::mutex>(segment_locks[i]);
    }

    gc.collect(root_set, heap_memory);
    coalesce_segments();
}

size_t heap_manager::get_segment_category_index(size_t segment_index) const noexcept {
    if(segment_index < SMALL_OBJECT_SEGMENTS){
        return segment_index;
    } 
    else if(segment_index < SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS){
        return segment_index - SMALL_OBJECT_SEGMENTS;
    }
    else{
        return segment_index - SMALL_OBJECT_SEGMENTS - MEDIUM_OBJECT_SEGMENTS;
    }
}

segment& heap_manager::get_segment(size_t segment_index){
    if(segment_index < SMALL_OBJECT_SEGMENTS){
        return heap_memory.get_small_object_segment(segment_index);
    }
    else if(segment_index < SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS){
        return heap_memory.get_medium_object_segment(segment_index - SMALL_OBJECT_SEGMENTS);
    }
    else{
        return heap_memory.get_large_object_segment(segment_index - SMALL_OBJECT_SEGMENTS - MEDIUM_OBJECT_SEGMENTS);
    }
}

int heap_manager::find_suitable_segment(uint32_t bytes) noexcept {
    size_t start_idx{}, end_idx{};
    std::atomic<size_t>* last_segment_idx;
    int fallback_segment_idx = -1;
    uint32_t fallback_segment_size = 0;

    if(bytes <= SMALL_OBJECT_THRESHOLD){
        start_idx = 0;
        end_idx = SMALL_OBJECT_SEGMENTS;
        last_segment_idx = &last_small_segment;
    }
    else if(bytes <= MEDIUM_OBJECT_THRESHOLD){
        start_idx = SMALL_OBJECT_SEGMENTS;
        end_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS;
        last_segment_idx = &last_medium_segment;
    }
    else {
        start_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS;
        end_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + LARGE_OBJECT_SEGMENTS;
        last_segment_idx = &last_large_segment;
    }

    const size_t segment_count = end_idx - start_idx;
    size_t last_used = last_segment_idx->load(std::memory_order_acquire); 
    size_t start_offset = (last_used >= start_idx && last_used < end_idx) ? (last_used - start_idx) : 0;

    for(size_t offset = 0; offset < segment_count; ++offset){
        size_t relative_idx = (start_offset + offset + 1) % segment_count;
        size_t idx = start_idx + relative_idx;

        const segment_info* seg_info = free_memory_table.get_segment_info(idx);
        if(!seg_info) continue;

        const uint32_t free_bytes = std::atomic_ref<const uint32_t>(seg_info->free_bytes).load(std::memory_order_acquire);
        if(free_bytes < bytes + sizeof(header)) continue;
        
        if(fallback_segment_idx == -1 || fallback_segment_size < free_bytes){
            fallback_segment_idx = idx;
            fallback_segment_size = free_bytes;
        }

        std::unique_lock<std::mutex> segment_lock(segment_locks[idx], std::try_to_lock);
        if(!segment_lock.owns_lock()) continue;

        last_segment_idx->store(idx, std::memory_order_release);
        return static_cast<int>(idx);
    }

    if(fallback_segment_idx != -1){
        last_segment_idx->store(static_cast<size_t>(fallback_segment_idx), std::memory_order_release);
    }

    return fallback_segment_idx;
}

header* heap_manager::allocate_from_segment(size_t segment_index, uint32_t bytes){
    segment_info* seg_info = free_memory_table.get_segment_info(segment_index);
    if(!seg_info || !seg_info->free_list_head){
        return nullptr;
    }

    header* current = seg_info->free_list_head;
    header* prev = nullptr;

    while(current){
        if(current->is_free() && current->size >= bytes){
            break;
        }
        prev = current;
        current = current->next;
    }

    if(!current){
        return nullptr;
    }

    uint32_t remaining = current->size - bytes;
    if(remaining >= static_cast<uint32_t>(sizeof(header)) + 16){
        header* new_header = reinterpret_cast<header*>(reinterpret_cast<uint8_t*>(current) + sizeof(header) + static_cast<size_t>(bytes));
        
        new_header->size = remaining - static_cast<uint32_t>(sizeof(header));
        new_header->next = current->next;
        new_header->set_free(true);
        new_header->set_marked(false);

        current->size = bytes;
        current->next = new_header;
    }

    current->set_free(false);
    current->set_marked(false);

    if(prev){
        prev->next = current->next;
    }
    else{
        seg_info->free_list_head = current->next;
    }
    current->next = nullptr;

    seg_info->free_bytes -= (current->size + static_cast<uint32_t>(sizeof(header)));
    return current;
}

void heap_manager::coalesce_segment(size_t segment_index){
    segment& seg = get_segment(segment_index);
    segment_info* seg_info = free_memory_table.get_segment_info(segment_index);

    if(!seg_info) return;

    header* free_list = nullptr;
    uint32_t free_bytes = 0;

    uint8_t* current_ptr = seg.segment_memory;
    uint8_t* end_ptr = seg.segment_memory + SEGMENT_SIZE;

    while(current_ptr + sizeof(header) <= end_ptr){
        header* hdr = reinterpret_cast<header*>(current_ptr);
        if(hdr->size == 0 || current_ptr + sizeof(header) + static_cast<size_t>(hdr->size) > end_ptr){
            break;
        }

        uint8_t* next_ptr = current_ptr + sizeof(header) + static_cast<size_t>(hdr->size);
        while(next_ptr + sizeof(header) <= end_ptr){
            header* next_hdr = reinterpret_cast<header*>(next_ptr);
            if(!hdr->is_free() || !next_hdr->is_free()){
                break;
            }
            hdr->size += static_cast<uint32_t>(sizeof(header)) + next_hdr->size;
            next_ptr = current_ptr + sizeof(header) + static_cast<size_t>(hdr->size);
        }

        if(hdr->is_free()){
            hdr->next = free_list;
            free_list = hdr;
            free_bytes += hdr->size + sizeof(header);
        }

        current_ptr = current_ptr + sizeof(header) + static_cast<size_t>(hdr->size);
    }

    seg_info->free_list_head = free_list;
    std::atomic_ref<uint32_t>(seg_info->free_bytes).store(free_bytes, std::memory_order_release);
}

void heap_manager::coalesce_segments(){
    if constexpr (TOTAL_SEGMENTS == 0) return;
    
    std::latch completion_latch{TOTAL_SEGMENTS};

    for(size_t i = 0; i < TOTAL_SEGMENTS; ++i){
        heap_manager_thread_pool.enqueue([this, i, &completion_latch] -> void {
            coalesce_segment(i);
            completion_latch.count_down();
        });
    }

    completion_latch.wait();
}