#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>
#include <atomic>

/// is_free flag is on the lowest bit.
constexpr uint8_t IS_FREE = 0x01;

/// is marked flag is on the second lowest bit.
constexpr uint8_t IS_MARKED = 0x02;

/**
 * @struct header
 * @brief header of the block inside of the heap segment.
 * Occupies 16 bytes.
*/
struct header {
    /// if current block is free => pointer to the next free block; otherwise nullptr.
    header* next;
    /// size - the amount of memory the current block occupies.
    uint32_t size;
    /// flags - 0x000000mf; m - marked (0/1), f - free (0/1).
    std::atomic<uint32_t> flags; //< 32b only because of the alignment.

    /**
     * @brief creates the instance of the header.
     * @details sets next to nullptr, size to 0, flags to non-marked, free.
    */
    header();

    /**
     * @brief checks if the header is free.
     * @returns true if header has free flag 1, false otherwise
    */
    bool is_free() const noexcept;

    /**
     * @brief checks if the header is marked.
     * @returns true if header has marked flag 1, false otherwise.
    */
    bool is_marked() const noexcept;

    /** 
     * @brief sets the is_free flag.
     * @param free - value for the is_free flag.
     * @example free==true, flags = 0x00000000 => flags = 0x00000000 | 0x01 (0x00000001) => flags = 0x01.
     * @example free==false, flags = 0x00000001 => flags = 0x00000001 & ~0x01 (0x11111110) => flags = 0x00.
    */
    void set_free(bool free) noexcept;

    /** 
     * @brief sets the is_marked flag.
     * @param marked - value for the is_marked flag.
     * @example marked==true, flags = 0x00000000 => flags = 0x00000000 | 0x02 (0x00000010) => flags = 0x02.
     * @example marked==false, flags = 0x00000010 => flags = 0x00000010 & ~0x02 (0x11111101) => flags = 0x00.
    */
    void set_marked(bool marked) noexcept;

    /**
     * @brief getter for the address where data begins.
     * @returns pointer to data.
    */
    void* data_ptr() noexcept;

    /**
     * @brief getter for the address where data begins.
     * @returns pointer to const data.
    */
    const void* data_ptr() const noexcept;

    /**
     * @brief getter for the header of the data.
     * @param ptr - pointer to data.
     * @returns pointer to header.
    */
    static header* from_data(void* ptr) noexcept;

    /**
     * @brief getter for the header of the data.
     * @param ptr - const pointer to data.
     * @returns const pointer to header.
    */
    static const header* from_data(const void* ptr) noexcept;

};

static_assert(sizeof(header) == 16, "Header must be 16B");

#endif