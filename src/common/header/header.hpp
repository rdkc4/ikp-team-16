#ifndef HEADER_HPP
#define HEADER_HPP

#include <cstdint>

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
    void* next;
    /// size - the amount of memory the current block occupies.
    uint32_t size;
    /// flags - 0x000000mf; m - marked (0/1), f - free (0/1).
    uint32_t flags; //< 32b only because of the alignment.

    header() : next{ nullptr }, size{ 0 }, flags{ 0 } {}

    bool is_free() const noexcept { return flags & IS_FREE; }
    bool is_marked() const noexcept { return flags & IS_MARKED; }

    /** 
     * @brief sets the is_free flag.
     * @param free - value for the is_free flag.
     * @example free==true, flags = 0x00000000 => flags = 0x00000000 | 0x01 (0x00000001) => flags = 0x01.
     * @example free==false, flags = 0x00000001 => flags = 0x00000001 & ~0x01 (0x11111110) => flags = 0x00.
    */
    void set_free(bool free) noexcept {
        flags = free ? (flags | IS_FREE) : (flags & ~IS_FREE);
    }

    /** 
     * @brief sets the is_marked flag.
     * @param marked - value for the is_marked flag.
     * @example marked==true, flags = 0x00000000 => flags = 0x00000000 | 0x02 (0x00000010) => flags = 0x02.
     * @example marked==false, flags = 0x00000010 => flags = 0x00000010 & ~0x02 (0x11111101) => flags = 0x00.
    */
    void set_marked(bool marked) noexcept {
        flags = marked ? (flags | IS_MARKED) : (flags & ~IS_MARKED);
    }

    /**
     * @brief getter for the address where data begins.
     * @returns pointer to data.
    */
    void* data_ptr() noexcept { return reinterpret_cast<void*>(this + 1); }

    /**
     * @brief getter for the address where data begins.
     * @returns pointer to const data.
    */
    const void* data_ptr() const noexcept { return reinterpret_cast<const void*>(this + 1); }

    /**
     * @brief getter for the header of the data.
     * @param ptr - pointer to data.
     * @returns pointer to header.
    */
    static header* from_data(void* ptr) noexcept { return reinterpret_cast<header*>(ptr) - 1; }

    /**
     * @brief getter for the header of the data.
     * @param ptr - const pointer to data.
     * @returns const pointer to header.
    */
    static const header* from_data(const void* ptr) noexcept { return reinterpret_cast<const header*>(ptr) - 1; }

};

static_assert(sizeof(header) == 16, "Header must be 16B");

#endif