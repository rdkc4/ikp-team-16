#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include <cstdint>

// size of a single segment in bytes
constexpr uint32_t SEGMENT_SIZE = 16 * 1024 * 1024;

/**
 * @struct segment
 * @brief represents a single segment on the heap.
*/
struct segment {
    /// pointer to the segment's memory block.
    uint8_t* segment_memory;
    /// number of bytes that are free in segment.
    uint32_t free_memory;

    /**
     * @brief creates an instance of the segment.
     * @details allocates SEGMENT_SIZE bytes of memory.
     * @throws std::bad_alloc when memory allocation fails.
     */
    segment();

    /**
     * @brief deletes the segment.
     * @details frees the allocated memory.
    */
    ~segment();

    /// deleted copy constructor.
    segment(const segment&) = delete;

    /// deleted assignment operator.
    segment& operator=(const segment&) = delete;

    /**
     * @brief constructs new segment from an existing one.
     * @param other - rvalue of the existing segment.
     * @details moves ownership of the data from other to this.
    */
    segment(segment&& other) noexcept;

    /**
     * @brief constructs new segment by assigning it an existing one.
     * @param other - rvalue of the existing segment.
     * @details moves ownership of the data from other to this.
    */
    segment& operator=(segment&& other) noexcept;

    /**
     * @brief initializes the free memory and sets initial header.
    */
    void initialize();

};

#endif