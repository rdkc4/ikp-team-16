#ifndef HEAP_HPP
#define HEAP_HPP

#include <cstddef>
#include <stdexcept>

#include "../common/segment/segment.hpp"

/// number of small object segments.
constexpr size_t SMALL_OBJECT_SEGMENTS = 4;

/// number of medium object segments.
constexpr size_t MEDIUM_OBJECT_SEGMENTS = 2;

/// number of large object segments.
constexpr size_t LARGE_OBJECT_SEGMENTS = 2;

/// total number of segments.
constexpr size_t TOTAL_SEGMENTS = SMALL_OBJECT_SEGMENTS + MEDIUM_OBJECT_SEGMENTS + LARGE_OBJECT_SEGMENTS;

/**
 * @class heap
 * @brief implementation of the segmented heap.
*/
class heap {
private:
    /// segments for small object allocation.
    segment small_object_segments[SMALL_OBJECT_SEGMENTS];
    
    /// segments for medium object allocation.
    segment medium_object_segments[MEDIUM_OBJECT_SEGMENTS];
    
    /// segments for large object allocation.
    segment large_object_segments[LARGE_OBJECT_SEGMENTS];

public:
    /**
     * @brief creates the instance of the heap.
     * @details initializes all segments.
    */
    heap() = default;

    /**
     * @brief deletes the heap object.
     * @details frees all segments.
    */
    ~heap() = default;

    /// deleted copy constructor.
    heap(const heap&) = delete;
    
    /// deleted assignment operator.
    heap& operator=(const heap&) = delete;

    /// deleted move constructor.
    heap(heap&&) = delete;

    /// deleted move assignment operator.
    heap& operator=(heap&&) = delete;

    /**
     * @brief getter for small object segments.
     * @param index - index of the small object segment.
     * @returns reference to a small object segment.
    */
    segment& get_small_object_segment(size_t index) {
        if(index >= SMALL_OBJECT_SEGMENTS) {
            throw std::out_of_range("Small object segment index out of range");
        }
        return small_object_segments[index];
    }

    /**
     * @brief getter for small object segments.
     * @param index - index of the small object segment.
     * @returns const reference to a small object segment.
    */
    const segment& get_small_object_segment(size_t index) const {
        if(index >= SMALL_OBJECT_SEGMENTS) {
            throw std::out_of_range("Small object segment index out of range");
        }
        return small_object_segments[index];
    }

    /**
     * @brief getter for medium object segments.
     * @param index - index of the medium object segment.
     * @returns reference to a medium object segment.
    */
    segment& get_medium_object_segment(size_t index) {
        if(index >= MEDIUM_OBJECT_SEGMENTS) {
            throw std::out_of_range("Medium object segment index out of range");
        }
        return medium_object_segments[index];
    }

    /**
     * @brief getter for medium object segments.
     * @param index - index of the medium object segment.
     * @returns const reference to a medium object segment.
    */
    const segment& get_medium_object_segment(size_t index) const {
        if(index >= MEDIUM_OBJECT_SEGMENTS) {
            throw std::out_of_range("Medium object segment index out of range");
        }
        return medium_object_segments[index];
    }

    /**
     * @brief getter for large object segments.
     * @param index - index of the large object segment.
     * @returns reference to a large object segment.
    */
    segment& get_large_object_segment(size_t index) {
        if(index >= LARGE_OBJECT_SEGMENTS) {
            throw std::out_of_range("Large object segment index out of range");
        }
        return large_object_segments[index];
    }

    /**
     * @brief getter for large object segments.
     * @param index - index of the large object segment.
     * @returns const reference to a large object segment.
    */
    const segment& get_large_object_segment(size_t index) const {
        if(index >= LARGE_OBJECT_SEGMENTS) {
            throw std::out_of_range("Large object segment index out of range");
        }
        return large_object_segments[index];
    }
};

#endif