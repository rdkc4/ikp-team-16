#ifndef HEAP_MANAGER_HPP
#define HEAP_MANAGER_HPP

#include <cstdint>
#include <atomic>
#include <latch>
#include <memory>
#include <mutex>

#include "../heap/heap.hpp"
#include "../segment-free-memory-table/segment-free-memory-table.hpp"
#include "../root-set-table/root-set-table.hpp"
#include "../garbage-collector/gc.hpp"

/// maximum small object size in bytes (up to 256B).
constexpr uint32_t SMALL_OBJECT_THRESHOLD = 256;

/// maximum medium object size in bytes (up to 2KB).
constexpr uint32_t MEDIUM_OBJECT_THRESHOLD = 2 * 1024;

/// maximum large object size in bytes (up to 256KB).
constexpr uint32_t LARGE_OBJECT_THRESHOLD = 256 * 1024;

/**
 * @class heap_manager
 * @brief manages the memory on the heap.
*/
class heap_manager {
private:
    /// segmented memory for object allocation.
    heap heap_memory;

    /// table containing the linked lists of free memory for each segment.
    segment_free_memory_table free_memory_table;
   
    /// table containing the roots. 
    root_set_table root_set;

    /// thread pool for coalescing segments.
    thread_pool heap_manager_thread_pool;

    /// gc for heap cleanup.
    garbage_collector gc;

    /// locks for heap segments.
    std::mutex segment_locks[TOTAL_SEGMENTS];

    /// indicates whether gc is currently running.
    std::atomic<bool> gc_in_progress{false};

    /// small object segment that was used last, default to last.
    std::atomic<size_t> last_small_segment{SMALL_OBJECT_SEGMENTS - 1};

    /// medium object segment that was used last, default to last.
    std::atomic<size_t> last_medium_segment{SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS - 1};

    /// large object segment that was used last, default to last.
    std::atomic<size_t> last_large_segment{SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + LARGE_OBJECT_SEGMENTS - 1};
    
    /**
     * @brief getter for the index of the segment based on object size category.
     * @param segment_index - index of the segment 0 to (n-1).
     * @returns index of the segment in an object category.
    */
    size_t get_segment_category_index(size_t segment_index) const noexcept {
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

    /**
     * @brief getter for the segment based on index.
     * @param segment_index - index of the segment.
     * @returns reference to a segment.
    */
    segment& get_segment(size_t segment_index){
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

    /**
     * @brief finds a segment that can store required bytes.
     * @param bytes - number of bytes that need to be allocated.
     * @returns index of the segment if segment can allocate enough bytes, -1 otherwise.
    */
    int find_suitable_segment(uint32_t bytes) noexcept {
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

    /**
     * @brief allocates object on the heap segment.
     * @param segment_index - index of the segment.
     * @param bytes - required memory.
     * @returns pointer to the header of the object.
    */
    header* allocate_from_segment(size_t segment_index, uint32_t bytes){
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

    /**
     * @brief merges free blocks on the segment.
     * @param segment_index - index of the segment. 
    */
    void coalesce_segment(size_t segment_index){
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

    /**
     * @brief merges free blocks of segments.
     * @warning must be called during the STW, after gc finishes collecting.
    */
    void coalesce_segments(){
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

public:
    /**
     * @brief creates the instance of the heap manager.
     * @param gc_thread_count - size of gc thread pool, defaults to 1.
     * @details initializes the segments on the heap, initializes free memory tables.
    */
    heap_manager(size_t hm_thread_count, size_t gc_thread_count = 1) : heap_manager_thread_pool(hm_thread_count), gc(gc_thread_count) {
        for(size_t i = 0; i < SMALL_OBJECT_SEGMENTS; ++i) {
            segment& segment = heap_memory.get_small_object_segment(i);
            header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
            free_memory_table.update_segment(i, segment.free_memory, initial_header);
        }

        for(size_t i = 0; i < MEDIUM_OBJECT_SEGMENTS; ++i) {
            segment& segment = heap_memory.get_medium_object_segment(i);
            header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
            free_memory_table.update_segment(SMALL_OBJECT_SEGMENTS + i, segment.free_memory, initial_header);
        }

        for(size_t i = 0; i < LARGE_OBJECT_SEGMENTS; ++i) {
            segment& segment = heap_memory.get_large_object_segment(i);
            header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
            free_memory_table.update_segment(SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + i, segment.free_memory, initial_header);
        }
    }

    /**
     * @brief deletes the instance of the heap manager.
    */
    ~heap_manager() = default;

    /// deleted copy constructor.
    heap_manager(const heap_manager&) = delete;

    /// deleted assignment operator.
    heap_manager& operator=(const heap_manager&) = delete;

    /// deleted move constructor.
    heap_manager(heap_manager&&) = delete;

    /// deleted move assignment operator.
    heap_manager& operator=(heap_manager&&) = delete;

    /**
     * @brief tries to allocate memory on the heap.
     * @param bytes - number of bytes that need to be allocated.
     * @returns pointer to header of the object if allocation is successful, nullptr otherwise.
    */
    header* allocate(uint32_t bytes){
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

    /**
     * @brief adds new root to a root-set-table.
     * @param key - name of the root.
     * @param base - element of the root-set-table.
    */
    void add_root(std::string key, std::unique_ptr<root_set_base> base){
        root_set.add_root(std::move(key), std::move(base));
    }

    root_set_base* get_root(const std::string& key) {
        return root_set.get_root(key);
    }

    /**
     * @brief removes root from the root-set-table.
     * @param key - const reference to a name of the root-set-table.
    */
    void remove_root(const std::string& key){
        root_set.remove_root(key);
    }

    /**
     * @brief starts the garbage collection.
     * @details "Stop the world", mark & sweep collection and coalescing of segments.
     * @warning can be called by client, but it may be expensive if called frequently.
    */
    void collect_garbage(){
        std::unique_lock<std::mutex> locks[TOTAL_SEGMENTS];

        for(size_t i = 0; i < TOTAL_SEGMENTS; ++i){
            locks[i] = std::unique_lock<std::mutex>(segment_locks[i]);
        }

        gc.collect(root_set, heap_memory);
        coalesce_segments();
    }
};

#endif