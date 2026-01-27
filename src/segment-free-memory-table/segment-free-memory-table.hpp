#ifndef SEGMENT_FREE_MEMORY_TABLE_HPP
#define SEGMENT_FREE_MEMORY_TABLE_HPP

#include <cstddef>
#include "../common/segment/segment_info.hpp"
#include "../common/hash-map/hash-map.hpp"
#include "../common/header/header.hpp"

/**
 * @class segment_free_memory_table
 * @brief table containing the information of all segments.
*/
class segment_free_memory_table {
private:
    /// maps id of the segment to its information.
    hash_map<size_t, segment_info> free_mem_table;

public:
    /**
     * @brief creates the instance of the segment free memory table.
    */
    segment_free_memory_table() = default;

    /**
     * @brief deletes the segment free memory table.
    */
    ~segment_free_memory_table() = default;

    /// deleted copy constructor.
    segment_free_memory_table(const segment_free_memory_table&) = delete;
    
    /// deleted assignment operator.
    segment_free_memory_table& operator=(const segment_free_memory_table&) = delete;

    /**
     * @brief creates the instance of the segment free memory table from an existing one
     * @param other - rvalue of the segment free memory table.
    */
    segment_free_memory_table(segment_free_memory_table&& other) noexcept = default;
    
    /**
     * @brief assigns the instance of the segment free memory table from an existing one
     * @param other - rvalue of the segment free memory table.
    */
    segment_free_memory_table& operator=(segment_free_memory_table&& other) noexcept = default;

    /**
     * @brief inserts or updates a segment.
     * @param segment_index - index of the segment.
     * @param free_bytes - free bytes in a segment.
     * @param free_list_head - pointer to a head of the free list.
    */
    void update_segment(size_t segment_index, uint32_t free_bytes, header* free_list_head) {
        free_mem_table.insert(segment_index, segment_info(free_bytes, free_list_head));
    }

    /**
     * @brief getter for the info of the specific segment.
     * @param segment_index - index of the segment.
     * @returns pointer to a segment info.
    */
    segment_info* get_segment_info(size_t segment_index) noexcept {
        return free_mem_table.find(segment_index);
    }

    /**
     * @brief getter for the info of the specific segment.
     * @param segment_index - index of the segment.
     * @returns const pointer to a segment info.
    */
    const segment_info* get_segment_info(size_t segment_index) const noexcept {
        return free_mem_table.find(segment_index);
    }

    /**
     * @brief removes the segment from the table.
     * @param segment_index - index of the segment.
    */
    void remove_segment(size_t segment_index) {
        free_mem_table.erase(segment_index);
    }

    /**
     * @brief removes all segments from the table.
    */
    void clear_segments() noexcept {
        free_mem_table.clear();
    }

    /**
     * @brief getter for the number of segments in the table.
     * @returns number of segments in the table.
    */
    size_t segment_count() const noexcept {
        return free_mem_table.get_size();
    }

};

#endif