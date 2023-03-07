//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.h
//
// Identification: src/include/buffer/lru_replacer.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <mutex>  // NOLINT
#include <vector>

#include "buffer/replacer.h"
#include "common/config.h"
#include <deque>
#include <vector>

namespace bustub {


class FrameInfo {
  public:
    
    FrameInfo(frame_id_t id);

    ~FrameInfo();
    
    void IncPins();
    
    void DecPins();

    frame_id_t GetFrameID();

    unsigned int GetNumPins();
  
  private:
    unsigned int pins;

    frame_id_t frame_id;
};




/**
 * LRUReplacer implements the Least Recently Used replacement policy.
 */
class LRUReplacer : public Replacer {
 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit LRUReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~LRUReplacer() override;

  auto Victim(frame_id_t *frame_id) -> bool override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  auto Size() -> size_t override;

  void Delete(frame_id_t frame_id);


 private:
  std::mutex replacer_mutex;

  std::deque<FrameInfo*> lru_queue;

  std::vector<FrameInfo*> used_vec;

  FrameInfo *GetFrameInfoUsed(frame_id_t frame_id);

  FrameInfo *GetFrameInfoQueue(frame_id_t frame_id);

  void AddUsed(FrameInfo *frame_info);

  void RemoveUsed(frame_id_t frame_id);

  void PushQueue(FrameInfo *frame_info);

  FrameInfo *PopQueue();

  void RemoveQueue(frame_id_t frame_id);
};

}  // namespace bustub
