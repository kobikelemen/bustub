//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) {
    for (size_t i=0; i < num_pages; i++) {
        FrameInfo *frame_info = new FrameInfo(i);
        AddUsed(frame_info);
    }
}

LRUReplacer::~LRUReplacer() {
    for (auto frame_info : used_vec) {
        delete frame_info;
    }
}

auto LRUReplacer::Victim(frame_id_t *frame_id) -> bool {
    std::scoped_lock lock{replacer_mutex};
    if (Size() > 0) {
        FrameInfo *frame_info = PopQueue();
        *frame_id = frame_info->GetFrameID();
        return true;
    } else {
        frame_id = nullptr;
        return false;
    }
}

void LRUReplacer::Pin(frame_id_t frame_id) {
    std::scoped_lock lock{replacer_mutex};
    FrameInfo *frame_info = GetFrameInfoQueue(frame_id);
    if (!frame_info)
        frame_info = GetFrameInfoUsed(frame_id);
    if (!frame_info)
        return;
    frame_info->IncPins();
    if (frame_info->GetNumPins() == 1) {
        RemoveQueue(frame_info->GetFrameID());
        AddUsed(frame_info);
    }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
    std::scoped_lock lock{replacer_mutex};
    FrameInfo *frame_info = GetFrameInfoUsed(frame_id);
    if (!frame_info) 
        return;
    frame_info->DecPins();
    if (frame_info->GetNumPins() == 0) {
        RemoveUsed(frame_info->GetFrameID());
        PushQueue(frame_info);
    }
}

auto LRUReplacer::Size() -> size_t {
    return lru_queue.size();
}

void LRUReplacer::Delete(frame_id_t frame_id) {
    RemoveUsed(frame_id);
    RemoveQueue(frame_id);
}


void LRUReplacer::AddUsed(FrameInfo *frame_info) {
    used_vec.push_back(frame_info);
}

void LRUReplacer::RemoveUsed(frame_id_t frame_id) {
    for (unsigned int i=0; i < used_vec.size(); i++) {
        if (used_vec[i]->GetFrameID() == frame_id) {
            used_vec.erase(used_vec.begin() + i);
            return;
        }
    }
}

void LRUReplacer::PushQueue(FrameInfo *frame_info) {
    lru_queue.push_back(frame_info);
}

FrameInfo *LRUReplacer::PopQueue() {
    if (lru_queue.size() > 0) {
        FrameInfo *frame_info = lru_queue.front();
        lru_queue.pop_front();
        return frame_info;
    }
    return nullptr;
    
}

void LRUReplacer::RemoveQueue(frame_id_t frame_id) {
    // not pop_front, but remove from where ever located
    for (unsigned int i=0; i < lru_queue.size(); i++) {
        if (lru_queue[i]->GetFrameID() == frame_id) {
            lru_queue.erase(lru_queue.begin() + i);
            return;
        }
    }
}

FrameInfo *LRUReplacer::GetFrameInfoUsed(frame_id_t id) {
    for (auto frame_info : used_vec) {
        if (id == frame_info->GetFrameID()) {
            return frame_info;
        }
    }
    return nullptr;
}

FrameInfo *LRUReplacer::GetFrameInfoQueue(frame_id_t id) {
    for (auto frame_info : lru_queue) {
        if (id == frame_info->GetFrameID()) {
            return frame_info;
        }
    }
    return nullptr;
}

FrameInfo::FrameInfo(frame_id_t id) {
    frame_id = id;
    pins = 0;
}

FrameInfo::~FrameInfo() {}

void FrameInfo::IncPins() {
    pins++;
}

void FrameInfo::DecPins() {
    if (pins > 0) {
        pins--;
    }
}

frame_id_t FrameInfo::GetFrameID() {
    return frame_id;
}

unsigned int FrameInfo::GetNumPins() {
    return pins;
}

}  // namespace bustub


