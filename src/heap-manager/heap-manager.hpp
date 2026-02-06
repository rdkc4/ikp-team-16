#ifndef HEAP_MANAGER_HPP
#define HEAP_MANAGER_HPP

#include <cstdint>
#include <atomic>
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
    /// locks for heap segments.
    std::mutex segment_locks[TOTAL_SEGMENTS];

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
    size_t get_segment_category_index(size_t segment_index) const noexcept;

    /**
     * @brief getter for the segment based on index.
     * @param segment_index - index of the segment.
     * @returns reference to a segment.
    */
    segment& get_segment(size_t segment_index);

    /**
     * @brief finds a segment that can store required bytes.
     * @param bytes - number of bytes that need to be allocated.
     * @returns index of the segment if segment can allocate enough bytes, -1 otherwise.
    */
    int find_suitable_segment(uint32_t bytes) noexcept;

    /**
     * @brief allocates object on the heap segment.
     * @param segment_index - index of the segment.
     * @param bytes - required memory.
     * @returns pointer to the header of the object.
    */
    header* allocate_from_segment(size_t segment_index, uint32_t bytes);

    /**
     * @brief merges free blocks on the segment.
     * @param segment_index - index of the segment. 
    */
    void coalesce_segment(size_t segment_index);

    /**
     * @brief merges free blocks of segments.
     * @warning must be called during the STW, after gc finishes collecting.
    */
    void coalesce_segments();

public:
    /**
     * @brief creates the instance of the heap manager.
     * @param gc_thread_count - size of gc thread pool, defaults to 1.
     * @details initializes the segments on the heap, initializes free memory tables.
    */
    heap_manager(size_t hm_thread_count, size_t gc_thread_count = 1);

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
    header* allocate(uint32_t bytes);

    /**
     * @brief adds new root to a root-set-table.
     * @param key - name of the root.
     * @param base - element of the root-set-table.
    */
    void add_root(std::string key, std::unique_ptr<root_set_base> base);

    /**
     * @brief getter for the root from the root-set-table.
     * @param key - name of the root.
     * @returns pointer to a root.
    */
    root_set_base* get_root(const std::string& key);

    /**
     * @brief removes root from the root-set-table.
     * @param key - const reference to a name of the root-set-table.
    */
    void remove_root(const std::string& key);

    /**
     * @brief starts the garbage collection.
     * @details "Stop the world", mark & sweep collection and coalescing of segments.
     * @warning can be called by client, but it may be expensive if called frequently.
    */
    void collect_garbage();

};

#endif