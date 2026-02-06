#include "header.hpp"

header::header() : next{ nullptr }, size{ 0 }, flags{ 0x01 } {}

bool header::is_free() const noexcept {
    return flags.load(std::memory_order_acquire) & IS_FREE; 
}

bool header::is_marked() const noexcept { 
    return flags.load(std::memory_order_acquire) & IS_MARKED; 
}

void header::set_free(bool free) noexcept {
    if(free){
        flags.fetch_or(IS_FREE, std::memory_order_release);
    }
    else {
        flags.fetch_and(~IS_FREE, std::memory_order_release);
    }
}

void header::set_marked(bool marked) noexcept {
    if(marked){
        flags.fetch_or(IS_MARKED, std::memory_order_release);
    }
    else {
        flags.fetch_and(~IS_MARKED, std::memory_order_release);
    }
}

void* header::data_ptr() noexcept {
    return reinterpret_cast<void*>(this + 1);
}

const void* header::data_ptr() const noexcept {
    return reinterpret_cast<const void*>(this + 1);
}

header* header::from_data(void* ptr) noexcept {
    return reinterpret_cast<header*>(ptr) - 1;
}

const header* header::from_data(const void* ptr) noexcept {
    return reinterpret_cast<const header*>(ptr) - 1;
}