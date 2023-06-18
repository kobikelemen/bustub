//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"

#include "common/exception.h"
#include "common/macros.h"
#include "storage/page/page_guard.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, size_t replacer_k,
                                     LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // we allocate a consecutive memory space for the buffer pool
  pages_ = new Page[pool_size_];
  replacer_ = std::make_unique<LRUReplacer>(pool_size);
  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}


BufferPoolManager::~BufferPoolManager() { delete[] pages_; }


void BufferPoolManager::GetReplaceFrameId(frame_id_t *frame_id) {
  if (free_list_.size() > 0) {
    *frame_id = free_list_.back();
    free_list_.pop_back();
  } else if (replacer_->Size() > 0) {
    replacer_->Victim(frame_id);
  } else {
    *frame_id = -1;
  }
}


void BufferPoolManager::WritePageToDisk(page_id_t page_id) {
  frame_id_t frame_id = page_table_[page_id];
  pages_[frame_id].WLatch();
  disk_manager_->WritePage(page_id, pages_[frame_id].GetData());
  /* Written out to disk so delete from page table. */
  page_table_.erase(page_id);
  pages_[frame_id].WUnlatch();
  
}


void BufferPoolManager::ReadPageFromDisk(page_id_t read_page_id, page_id_t replace_page_id) {
  frame_id_t replace_frame_id = page_table_[replace_page_id];
  pages_[replace_frame_id].RLatch();
  disk_manager_->ReadPage(read_page_id, pages_[replace_frame_id].GetData());
  page_table_[read_page_id] = replace_frame_id;
  pages_[replace_frame_id].RUnlatch();
}


auto BufferPoolManager::NewPage(page_id_t *page_id) -> Page * {
  frame_id_t free_frame_id;
  GetReplaceFrameId(&free_frame_id);
  
  if (free_frame_id == -1) {
    page_id = nullptr;
    return nullptr;
  } 
  /* Get page corresponding to free_frame_id */
  page_id_t new_page_id = AllocatePage();
  *page_id = new_page_id;
  /* Map newly allocated page id to frame  */
  page_table_[new_page_id] = free_frame_id;
  if (pages_[free_frame_id].IsDirty()) {
    /* Write back to disk */
    FlushPage(new_page_id);
  }
  /* Reset page */
  pages_[free_frame_id].Reset();
  PinPage(new_page_id);
  return &pages_[free_frame_id];
}


auto BufferPoolManager::FetchPage(page_id_t page_id, [[maybe_unused]] AccessType access_type) -> Page * {
  frame_id_t replace_frame_id;
  /* Check if required page is NOT in buffer bool. */
  if (page_table_.find(page_id) == page_table_.end()) {
    /* Replace a page with the page from disk. */
    GetReplaceFrameId(&replace_frame_id);
    if (replace_frame_id == -1) {
      return nullptr;
    }
    page_id_t replace_page_id = PageIdFromFrameId(replace_frame_id);
    
    if (pages_[replace_frame_id].IsDirty()) {
      FlushPage(replace_page_id);
    }
    ReadPageFromDisk(page_id, replace_page_id);
    PinPage(page_id);
    return &pages_[replace_frame_id];
  } else {
    /* Found page_id in page table hence in buffer pool. */
    replace_frame_id = page_table_[page_id];
    PinPage(page_id);
    return &pages_[replace_frame_id];
  } 
}

auto BufferPoolManager::PinPage(page_id_t page_id) -> bool {
  if (page_table_.find(page_id) == page_table_.end())
    return false;
  frame_id_t frame_id = page_table_[page_id];
  replacer_->Pin(frame_id);
  pages_[frame_id].IncPin();
  return true;
}


auto BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty, [[maybe_unused]] AccessType access_type) -> bool {
  /* Page not in buffer pool. */
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }
    
  frame_id_t frame_id = page_table_[page_id];
  if (pages_[frame_id].GetPinCount() == 0) {
    return false;
  }
    
  replacer_->Unpin(frame_id);
  pages_[frame_id].DecPin();
  return true;
}


auto BufferPoolManager::FlushPage(page_id_t page_id) -> bool {
  if (page_table_.find(page_id) == page_table_.end())
    return false;
  frame_id_t frame_id = page_table_[page_id];
  pages_[frame_id].SetDirty(false);
  WritePageToDisk(page_id);
  
  return true;
}


void BufferPoolManager::FlushAllPages() {
  for (size_t page_id = 0; page_id < pool_size_; page_id++) {
    FlushPage(page_id);
  }
}


auto BufferPoolManager::DeletePage(page_id_t page_id) -> bool {
  if (page_table_.find(page_id) == page_table_.end()) 
    return true;
  frame_id_t frame_id = page_table_[page_id];
  pages_[frame_id].WLatch();
  if (pages_[frame_id].GetPinCount() > 0) 
    return false;
  replacer_->Delete(frame_id);
  page_table_.erase(page_id);
  pages_[frame_id].Reset();
  free_list_.push_back(frame_id);
  DeallocatePage(page_id);
  pages_[frame_id].WUnlatch();
  return true;
}


auto BufferPoolManager::AllocatePage() -> page_id_t { return next_page_id_++; }


auto BufferPoolManager::FetchPageBasic(page_id_t page_id) -> BasicPageGuard {
  Page *page = FetchPage(page_id);
  BasicPageGuard page_guard = BasicPageGuard(this, page);
  return page_guard;
}


auto BufferPoolManager::FetchPageRead(page_id_t page_id) -> ReadPageGuard {
  Page *page = FetchPage(page_id);
  page->RLatch();
  ReadPageGuard page_guard = ReadPageGuard(this, page);
  return page_guard;
}


auto BufferPoolManager::FetchPageWrite(page_id_t page_id) -> WritePageGuard {
  Page *page = FetchPage(page_id);
  page->WLatch();
  WritePageGuard page_guard = WritePageGuard(this, page);
  return page_guard;
}


auto BufferPoolManager::NewPageGuarded(page_id_t *page_id) -> BasicPageGuard {
  Page *page = NewPage(page_id);
  BasicPageGuard page_guard = BasicPageGuard(this, page);
  return page_guard; 
}


page_id_t BufferPoolManager::PageIdFromFrameId(frame_id_t frame_id) {
  for (auto pair : page_table_) {
    if (pair.second == frame_id) {
      return pair.first;
    }
  }
  return -1;
}

}  // namespace bustub
