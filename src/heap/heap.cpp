#include "heap.hpp"

#include <stdexcept>

segment& heap::get_small_object_segment(size_t index) {
    if(index >= SMALL_OBJECT_SEGMENTS) {
        throw std::out_of_range("Small object segment index out of range");
    }
    return small_object_segments[index];
}

const segment& heap::get_small_object_segment(size_t index) const {
    if(index >= SMALL_OBJECT_SEGMENTS) {
        throw std::out_of_range("Small object segment index out of range");
    }
    return small_object_segments[index];
}

segment& heap::get_medium_object_segment(size_t index) {
    if(index >= MEDIUM_OBJECT_SEGMENTS) {
        throw std::out_of_range("Medium object segment index out of range");
    }
    return medium_object_segments[index];
}

const segment& heap::get_medium_object_segment(size_t index) const {
    if(index >= MEDIUM_OBJECT_SEGMENTS) {
        throw std::out_of_range("Medium object segment index out of range");
    }
    return medium_object_segments[index];
}

segment& heap::get_large_object_segment(size_t index) {
    if(index >= LARGE_OBJECT_SEGMENTS) {
        throw std::out_of_range("Large object segment index out of range");
    }
    return large_object_segments[index];
}

const segment& heap::get_large_object_segment(size_t index) const {
    if(index >= LARGE_OBJECT_SEGMENTS) {
        throw std::out_of_range("Large object segment index out of range");
    }
    return large_object_segments[index];
}