#ifndef SEGMENT_INFO_HPP
#define SEGMENT_INFO_HPP

#include <cstdint>

#include "../header/header.hpp"

/**
 * @struct segment_info
 * @brief representation of the element inside of the free_memory_table.
*/
struct segment_info {
    /// pointer to a head of the free list.
    header* free_list_head;

    /// number of free bytes in a segment.
    uint32_t free_bytes;

    /**
     * @brief creates the instance of the segment_info.
     * @details sets free_bytes to 0, free_list_head to nullptr
    */
    segment_info();

    /**
     * @brief creates the instance of the segment_info.
     * @param head - pointer to a head of the free memory list.
     * @param bytes - number of free bytes.
    */
    segment_info(header* head, uint32_t bytes);
};

#endif