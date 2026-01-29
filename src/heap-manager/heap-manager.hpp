#ifndef HEAP_MANAGER_HPP
#define HEAP_MANAGER_HPP

#include <cstdint>
#include <memory>

#include "../heap/heap.hpp"
#include "../segment-free-memory-table/segment-free-memory-table.hpp"
#include "../root-set-table/root-set-table.hpp"

/// maximum small object size in bytes.
constexpr uint32_t SMALL_OBJECT_THRESHOLD = 256;

/// maximum medium object size in bytes.
constexpr uint32_t MEDIUM_OBJECT_THRESHOLD = 2 * 1024;

/// maximum large object size in bytes.
constexpr uint32_t LARGE_OBJECT_THRESHOLD = 2 * 1024 * 1024;

/**
 * @class heap_manager
 * @brief manages the memory on the heap.
*/
class heap_manager {
private:
    /// segmented memory for object allocation.
    heap memory_heap;

    /// table containing the linked lists of free memory for each segment.
    segment_free_memory_table free_memory_table;
   
    /// table containing the roots. 
    root_set_table root_set;

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
            return memory_heap.get_small_object_segment(segment_index);
        }
        else if(segment_index < SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS){
            return memory_heap.get_medium_object_segment(segment_index - SMALL_OBJECT_SEGMENTS);
        }
        else{
            return memory_heap.get_large_object_segment(segment_index - SMALL_OBJECT_SEGMENTS - MEDIUM_OBJECT_SEGMENTS);
        }
    }

    /**
     * @brief finds a segment that can store required bytes.
     * @param bytes - number of bytes that need to be allocated.
     * @returns index of the segment if segment can allocate enough bytes, -1 otherwise.
    */
    int find_suitable_segment(uint32_t bytes) const noexcept {
        size_t start_idx{}, end_idx{};

        if(bytes <= SMALL_OBJECT_THRESHOLD){
            start_idx = 0;
            end_idx = SMALL_OBJECT_SEGMENTS;
        }
        else if(bytes <= MEDIUM_OBJECT_THRESHOLD){
            start_idx = SMALL_OBJECT_SEGMENTS;
            end_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS;
        }
        else {
            start_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS;
            end_idx = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + LARGE_OBJECT_SEGMENTS;
        }

        for(size_t i = start_idx; i < end_idx; ++i){
            const segment_info* seg_info = free_memory_table.get_segment_info(i);
            if(seg_info && seg_info->free_bytes >= bytes + sizeof(header)){
                return static_cast<int>(i);
            }
        }

        return -1;
    }

    /**
     * @brief allocates object on the heap segment.
     * @param segment_index - index of the segment.
     * @param bytes - required memory.
     * @returns pointer to data of the object.
    */
    void* allocate_from_segment(size_t segment_index, uint32_t bytes){
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
        return current->data_ptr();
    }

    /**
     * @brief merges free blocks on the segment.
     * @param segment_index - index of the segment. 
    */
    void coalesce_segment(size_t segment_index){
        segment& seg = get_segment(segment_index);
        segment_info* seg_info = free_memory_table.get_segment_info(segment_index);

        if(seg_info){
            return;
        }

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
        seg_info->free_bytes = free_bytes;
    }

public:
    /**
     * @brief creates the instance of the heap manager.
     * @details initializes the segments on the heap, initializes free memory tables.
    */
    heap_manager() {
        for(size_t i = 0; i < SMALL_OBJECT_SEGMENTS; ++i) {
            segment& segment = memory_heap.get_small_object_segment(i);
            header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
            free_memory_table.update_segment(i, segment.free_memory, initial_header);
        }

        for(size_t i = 0; i < MEDIUM_OBJECT_SEGMENTS; ++i) {
            segment& segment = memory_heap.get_medium_object_segment(i);
            header* initial_header = reinterpret_cast<header*>(segment.segment_memory);
            free_memory_table.update_segment(SMALL_OBJECT_SEGMENTS + i, segment.free_memory, initial_header);
        }

        for(size_t i = 0; i < LARGE_OBJECT_SEGMENTS; ++i) {
            segment& segment = memory_heap.get_large_object_segment(i);
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
     * @returns pointer to data if allocation is successful, nullptr otherwise.
    */
    void* allocate(uint32_t bytes){
        if(bytes == 0){
            return nullptr;
        }

        bytes = (bytes + 15) & ~15;

        int segment_index = find_suitable_segment(bytes);
        /// no suitable segments => call gc
        /*if(segment_index == -1){
            gc.collect_garbage();
            segment_index = find_suitable_segment(bytes);
            if(segment_index == -1){
                return nullptr;
            }
        }*/

        return allocate_from_segment(static_cast<size_t>(segment_index), bytes);
    }

    /**
     * @brief deallocates the object from the heap.
     * @param ptr - pointer to data of the object.
    */
    void deallocate(void* ptr){
        if(!ptr){
            return;
        }

        header* hdr = header::from_data(ptr);
        hdr->set_free(true);
        hdr->set_marked(false);
    }

    /**
     * @brief adds new root to a root-set-table.
     * @param key - name of the root.
     * @param base - element of the root-set-table.
    */
    void add_root_set(std::string key, std::unique_ptr<root_set_base> base){
        root_set.add_root(std::move(key), std::move(base));
    }

    root_set_base* get_root(std::string& key) {
        return root_set.get_root(key);
    }

    /**
     * @brief removes root from the root-set-table.
     * @param key - reference to a name of the root-set-table.
    */
    void remove_root_set(std::string& key){
        root_set.remove_root(key);
    }
};

#endif