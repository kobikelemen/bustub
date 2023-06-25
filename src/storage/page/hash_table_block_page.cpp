//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_table_block_page.cpp
//
// Identification: src/storage/page/hash_table_block_page.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "storage/page/hash_table_block_page.h"
#include "storage/index/generic_key.h"

namespace bustub {

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BLOCK_TYPE::Initialise() {
  memset(occupied_, 0x00, (BLOCK_ARRAY_SIZE - 1) / 8 + 1);
  memset(readable_, 0x00, (BLOCK_ARRAY_SIZE - 1) / 8 + 1);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BLOCK_TYPE::KeyAt(slot_offset_t bucket_ind) const -> KeyType {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || !readable_[bucket_ind])
    return {};
  return array_[bucket_ind].first;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BLOCK_TYPE::ValueAt(slot_offset_t bucket_ind) const -> ValueType {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || !readable_[bucket_ind])
    return {};
  return array_[bucket_ind].second;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BLOCK_TYPE::Insert(slot_offset_t bucket_ind, const KeyType &key, const ValueType &value) -> bool {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || occupied_[bucket_ind])
    return false;
  array_[bucket_ind] = {key, value};
  readable_[bucket_ind] = true;
  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_BLOCK_TYPE::Remove(slot_offset_t bucket_ind) {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || !occupied_[bucket_ind])
    return;
  array_[bucket_ind] = {};
  occupied_[bucket_ind] = 0;
  readable_[bucket_ind] = 0;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BLOCK_TYPE::IsOccupied(slot_offset_t bucket_ind) const -> bool {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || !occupied_[bucket_ind])
    return false;
  return true;
}

template <typename KeyType, typename ValueType, typename KeyComparator>
auto HASH_TABLE_BLOCK_TYPE::IsReadable(slot_offset_t bucket_ind) const -> bool {
  if (bucket_ind >= BLOCK_ARRAY_SIZE || bucket_ind < 0 || !readable_[bucket_ind])
    return false;
  return true;
}


/* SEVERAL METHODS NOT IMPLEMENTED FROM HEADER, E.G. NumReadable() */


// DO NOT REMOVE ANYTHING BELOW THIS LINE
template class HashTableBlockPage<int, int, IntComparator>;
template class HashTableBlockPage<GenericKey<4>, RID, GenericComparator<4>>;
template class HashTableBlockPage<GenericKey<8>, RID, GenericComparator<8>>;
template class HashTableBlockPage<GenericKey<16>, RID, GenericComparator<16>>;
template class HashTableBlockPage<GenericKey<32>, RID, GenericComparator<32>>;
template class HashTableBlockPage<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
