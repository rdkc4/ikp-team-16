#ifndef HEAP_HPP
#define HEAP_HPP

#include "../common/segment/segment.hpp"
#include <cstddef>

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

};

#endif