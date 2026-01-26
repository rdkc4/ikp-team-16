#ifndef SEGMENT_HPP
#define SEGMENT_HPP

#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

// size of a single segment in bytes
constexpr size_t SEGMENT_SIZE = 4 * 1024 * 1024;

/**
 * @struct segment
 * @brief represents a single segment on the heap.
*/
struct segment {
    /// pointer to the segment's memory block.
    uint8_t* segment_memory;
    /// number of bytes that are free in segment.
    size_t free_memory;

    /**
     * @brief creates an instance of the segment.
     * @details allocates SEGMENT_SIZE bytes of memory.
     * @throws std::bad_alloc when memory allocation fails.
     */
    segment(): 
        segment_memory(static_cast<uint8_t*>(::operator new(SEGMENT_SIZE, std::align_val_t{alignof(std::max_align_t)}))),
        free_memory(SEGMENT_SIZE) {}

    /**
     * @brief deletes the segment.
     * @details frees the allocated memory.
    */
    ~segment() {
        ::operator delete(segment_memory, std::align_val_t{alignof(std::max_align_t)});
    }

    /// deleted copy constructor.
    segment(const segment&) = delete;

    /// deleted assignment operator.
    segment& operator=(const segment&) = delete;

    /**
     * @brief constructs new segment from an existing one.
     * @param other - rvalue of the existing segment.
     * @details moves ownership of the data from other to this.
    */
    segment(segment&& other) noexcept :
        segment_memory(std::exchange(other.segment_memory, nullptr)),
        free_memory(std::exchange(other.free_memory, 0)) {}

    /**
     * @brief constructs new segment by assigning it an existing one.
     * @param other - rvalue of the existing segment.
     * @details moves ownership of the data from other to this.
    */
    segment& operator=(segment&& other) noexcept {
        if(this != &other){
            ::operator delete(segment_memory, std::align_val_t{alignof(std::max_align_t)});

            segment_memory = std::exchange(other.segment_memory, nullptr);
            free_memory = std::exchange(other.free_memory, 0);
        }
        return *this;
    }

};

#endif