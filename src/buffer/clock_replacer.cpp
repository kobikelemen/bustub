//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/clock_replacer.h"

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) {
    /* Each FrameInfo starts not being in clock. */
    for (size_t i=0; i < num_pages; i++) 
        clock_.push_back({false, false, i + 1});
    head_ = 0;
    num_pages_ = num_pages;
    size_ = 0;


}

ClockReplacer::~ClockReplacer() = default;


/* Loops through frame in clock until an element that is
   false is found, or until back to the start. If back to 
   start, no victim found so false returned. 'i' keeps
   track of if back to start. */
auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool { 
    size_t i = 0;
    while(i < num_pages_) {
        if (clock_[head_].in_clock_) {
            if (clock_[head_].ref_) {
                clock_[head_].ref_ = false;
                head_ ++;
                head_ = head_ % num_pages_;
                i ++;
            } else {
                /* Set frame_id to victim. */
                *frame_id = clock_[head_].frame_id_;
                clock_[head_].ref_ = true;
                size_ --;
                return true;
            }
        } else {
            /* If frame at head_ not in clock, skip forward one. */
            head_ ++;
            head_ = head_ % num_pages_;
        }
    }
    return false;
}

void ClockReplacer::Pin(frame_id_t frame_id) {
    if (clock_[frame_id - 1].in_clock_ == false || clock_[frame_id - 1].ref_ == false) {
        return;
    }
    clock_[frame_id - 1].in_clock_ = false;
    size_ --;
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
    if (clock_[frame_id - 1].in_clock_ == true) {
        return;
    }
    clock_[frame_id - 1].in_clock_ = true; 
    clock_[frame_id - 1].ref_ = true;
    size_ ++;
}

auto ClockReplacer::Size() -> size_t {
    return size_;
}

}  // namespace bustub
