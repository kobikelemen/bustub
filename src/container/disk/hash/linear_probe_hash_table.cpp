//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// linear_probe_hash_table.cpp
//
// Identification: src/container/hash/linear_probe_hash_table.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "container/disk/hash/linear_probe_hash_table.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_TYPE::LinearProbeHashTable(const std::string &name, BufferPoolManager *buffer_pool_manager,
                                      const KeyComparator &comparator, size_t num_buckets,
                                      HashFunction<KeyType> hash_fn)
    : buffer_pool_manager_(buffer_pool_manager), comparator_(comparator), hash_fn_(std::move(hash_fn)) 
{
  /* Initialise header page which stores mappings from block id to page id to find
     page that stores a given index. */
  header_page_id_ = INVALID_PAGE_ID;
  header_page_ = reinterpret_cast<HashTableHeaderPage*> (buffer_pool_manager_->NewPage(&header_page_id_)->GetDate());
  header_page_->SetPageId(header_page_id_);
  num_blocks_ = (size_t)(num_buckets / HEADER_NUM_IDS) + 1;
  header_page_->SetSize(num_blocks_);

  /* Initialises the array in header_page that maps block id to page id. */
  page_id_t block_page_id = INVALID_PAGE_ID;
  for (int i=0; i < num_blocks_; i++) {
    auto block = reinterpret_cast<HashTableBlockPage<KeyType, ValueType, KeyComparator>*> (buffer_pool_manager_->NewPage(&block_page_id)->GetData());
    block->Initialise();
    block->AddBlockPageId(block_page_id);
  }
}



/*****************************************************************************
 * SEARCH
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_TYPE::GetValue(Transaction *transaction, const KeyType &key, std::vector<ValueType> *result) -> bool {
  std::unordered_map<page_id_t, bool> seen_page_ids;
  auto index = hash_fn_(key);
  std::pair<page_id_t, size_t> page_id_offset = GetPageIdAndOffset(index);
  auto page_id = page_id_offset.first;
  auto offset = page_id_offset.second;
  auto block = reinterpret_cast<HashTableBlockPage<KeyType, ValueType, KeyComparator>*> (buffer_pool_manager_->FetchPage(page_id)->GetDate());
  /* This checks if the hashed offset is filled. If so, it goes to the next index, and
     to the next block if end of current block reached. Returns false if sees a
     page_id twice since this means hash table is full (returned to start). */
  while (true) {
    while (offset < BLOCK_ARRAY_SIZE) {
      if (block->KeyAt(offset) == key) {
        *result = {block->ValueAt(offset)};
        return true;
      }
      offset ++;
    }
    block_id ++;
    block_id = block_id % header_page_->GetMaxNumBlocks();
    page_id = header_page_->GetBlockPageId(block_id);
    if (seen_page_ids.find(page_id) != seen_page_ids.end())
      return false;
    block = reinterpret_cast<HashTableBlockPage*> (buffer_pool_manager_->FetchPage(&page_id)->GetDate());  
    offset = 0;
}



/*****************************************************************************
 * INSERTION
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_TYPE::Insert(Transaction *transaction, const KeyType &key, const ValueType &value) -> bool {
  std::unordered_map<page_id_t, bool> seen_page_ids;
  auto index = hash_fn_(key);
  std::pair<page_id_t, size_t> page_id_offset = GetPageIdAndOffset(index);
  auto page_id = page_id_offset.first;
  auto offset = page_id_offset.second;
  auto block = reinterpret_cast<HashTableBlockPage*> (buffer_pool_manager_->FetchPage(&page_id)->GetDate());
  /* This checks if the hashed offset is filled. If so, it goes to the next index, and
     to the next block if end of current block reached. Returns false if sees a
     page_id twice since this means hash table is full (returned to start). */
  while (true) {
    while (offset < BLOCK_ARRAY_SIZE) {
      if (block->IsOccupied(offset)) {
        block->Insert(offset, key, value);
        return true;
      }
      offset ++;
    }
    block_id ++;
    block_id = block_id % header_page_->GetMaxNumBlocks();
    page_id = header_page_->GetBlockPageId(block_id);
    if (seen_page_ids.find(page_id) != seen_page_ids.end())
      return false;
    block = reinterpret_cast<HashTableBlockPage*> (buffer_pool_manager_->FetchPage(&page_id)->GetDate());  
    offset = 0;
  }
}



/*****************************************************************************
 * REMOVE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_TYPE::Remove(Transaction *transaction, const KeyType &key, const ValueType &value) -> bool {
  std::unordered_map<page_id_t, bool> seen_page_ids;
  auto index = hash_fn_(key);
  std::pair<page_id_t, size_t> page_id_offset = GetPageIdAndOffset(index);
  auto page_id = page_id_offset.first;
  auto offset = page_id_offset.second;
  auto block = reinterpret_cast<HashTableBlockPage*> (buffer_pool_manager_->FetchPage(&page_id)->GetDate());
  /* This checks if the hashed offset is filled. If so, it goes to the next index, and
     to the next block if end of current block reached. Returns false if sees a
     page_id twice since this means hash table is full (returned to start). */
  while (true) {
    while (offset < BLOCK_ARRAY_SIZE) {
      if (block->KeyAt(offset) == key) {
        block->Remove(offset);
        return true;
      }
      offset ++;
    }
    block_id ++;
    block_id = block_id % header_page_->GetMaxNumBlocks();
    page_id = header_page_->GetBlockPageId(block_id);
    if (seen_page_ids.find(page_id) != seen_page_ids.end())
      return false;
    block = reinterpret_cast<HashTableBlockPage*> (buffer_pool_manager_->FetchPage(&page_id)->GetDate());  
    offset = 0;
  }
}

/*****************************************************************************
 * RESIZE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_TYPE::Resize(size_t initial_size) {}

/*****************************************************************************
 * GETSIZE
 *****************************************************************************/
template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_TYPE::GetSize() -> size_t {
  return 0;
}



template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_TYPE::GetPageIdAndOffset(size_t index) -> std::pair<page_id_t, size_t> {
  auto block_id = (size_t)(index / BLOCK_ARRAY_SIZE);
  auto page_id = header_page_->GetBlockPageId(block_id);
  seen_page_ids[page_id] = true;
  auto offset = block_id - (block_id * BLOCK_ARRAY_SIZE);
  return {page_id, offset};
}


template class LinearProbeHashTable<int, int, IntComparator>;

template class LinearProbeHashTable<GenericKey<4>, RID, GenericComparator<4>>;
template class LinearProbeHashTable<GenericKey<8>, RID, GenericComparator<8>>;
template class LinearProbeHashTable<GenericKey<16>, RID, GenericComparator<16>>;
template class LinearProbeHashTable<GenericKey<32>, RID, GenericComparator<32>>;
template class LinearProbeHashTable<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
