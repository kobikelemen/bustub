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
                clock_[head_].in_clock_ = false;
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

    int frame_index = GetFrameIndex(frame_id);
    if (frame_index == -1) {
        return;
    }

    if (clock_[frame_index].in_clock_ == false) {
        return;
    }
    clock_[frame_index].in_clock_ = false;
    size_ --;
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
    int frame_index = GetFrameIndex(frame_id);
    if (frame_index == -1) {
        return;
    }
    if (clock_[frame_index].in_clock_ == true) {
        return;
    }
    clock_[frame_index].in_clock_ = true; 
    clock_[frame_index].ref_ = true;
    size_t prev_index; 
    if (head_ == 0) 
        prev_index = num_pages_ - 1;
    else 
        prev_index = head_ - 1;
    
    MoveFrameIndex(frame_index, prev_index);
    size_ ++;
}


auto ClockReplacer::Size() -> size_t {
    return size_;
}

/* Moves the element of clock_ at index 'from_index' to 'to_index'. */
void ClockReplacer::MoveFrameIndex(size_t from_index, size_t to_index) {
    FrameInfo element = clock_[from_index];
    clock_.erase(clock_.begin() + from_index);
    clock_.insert(clock_.begin() + to_index, element);
}

/* Gets the index of frame with frame_id in clock_. */
int ClockReplacer::GetFrameIndex(frame_id_t frame_id) {
    for (int i = 0; (size_t)i < clock_.size(); i++) {
        if ((int)clock_[i].frame_id_ == frame_id) {
            
            return i;
        }
            
    }
    return -1;
}

}  // namespace bustub