#include "segment-info.hpp"

segment_info::segment_info() : free_list_head(nullptr), free_bytes(0) {}

segment_info::segment_info(header* head, uint32_t bytes) : free_list_head(head), free_bytes(bytes) {}