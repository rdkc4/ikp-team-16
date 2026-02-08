#include "segment.hpp"

#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

#include "../header/header.hpp"

segment::segment(): segment_memory(static_cast<uint8_t*>(::operator new(SEGMENT_SIZE, std::align_val_t{alignof(std::max_align_t)}))){
    initialize();
}

segment::~segment() {
    ::operator delete(segment_memory, std::align_val_t{alignof(std::max_align_t)});
}

segment::segment(segment&& other) noexcept : segment_memory(std::exchange(other.segment_memory, nullptr)), 
    free_memory(std::exchange(other.free_memory, 0)) {}

segment& segment::operator=(segment&& other) noexcept {
    if(this != &other){
        ::operator delete(segment_memory, std::align_val_t{alignof(std::max_align_t)});

        segment_memory = std::exchange(other.segment_memory, nullptr);
        free_memory = std::exchange(other.free_memory, 0);
    }
    return *this;
}

void segment::initialize() {
    header* hdr = new (segment_memory) header{};
    hdr->size = SEGMENT_SIZE - sizeof(header);
    free_memory = hdr->size;
}