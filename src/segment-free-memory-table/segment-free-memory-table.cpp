#include "segment-free-memory-table.hpp"

void segment_free_memory_table::update_segment(size_t segment_index, header* free_list_head, uint32_t free_bytes) {
    free_mem_table.insert(segment_index, segment_info(free_list_head, free_bytes));
}

segment_info* segment_free_memory_table::get_segment_info(size_t segment_index) noexcept {
    return free_mem_table.find(segment_index);
}

const segment_info* segment_free_memory_table::get_segment_info(size_t segment_index) const noexcept {
    return free_mem_table.find(segment_index);
}

void segment_free_memory_table::remove_segment(size_t segment_index) {
    free_mem_table.erase(segment_index);
}

void segment_free_memory_table::clear_segments() noexcept {
    free_mem_table.clear();
}

size_t segment_free_memory_table::segment_count() const noexcept {
    return free_mem_table.get_size();
}