#include "segment-info.hpp"

segment_info::segment_info() : free_bytes(0), free_list_head(nullptr) {}

segment_info::segment_info(uint32_t bytes, header* head) : free_bytes(bytes), free_list_head(head) {}