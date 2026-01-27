#ifndef SEGMENT_INFO_HPP
#define SEGMENT_INFO_HPP

#include <cstdint>

#include "../header/header.hpp"

/**
 * @struct segment_info
 * @brief representation of the element inside of the free_memory_table.
*/
struct segment_info {
    /// number of free bytes in a segment.
    uint32_t free_bytes;

    /// pointer to a head of the free list.
    header* free_list_head;

    /**
     * @brief creates the instance of the segment_info.
     * @details sets free_bytes to 0, free_list_head to nullptr
    */
    segment_info() : free_bytes(0), free_list_head(nullptr) {}

    /**
     * @brief creates the instance of the segment_info.
     * @param bytes - number of free bytes.
     * @param head - pointer to a head of the free memory list.
    */
    segment_info(uint32_t bytes, header* head) : free_bytes(bytes), free_list_head(head) {}
};

#endif