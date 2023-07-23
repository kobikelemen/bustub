//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/page/b_plus_tree_internal_page.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <iostream>
#include <sstream>

#include "common/exception.h"
#include "storage/page/b_plus_tree_internal_page.h"

namespace bustub {
/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, and set max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Init(int max_size) {
  memset(array_, 0, INTERNAL_PAGE_SIZE);
  SetSize(0);
  SetPageType(IndexPageType::INTERNAL_PAGE);
  SetMaxSize(INTERNAL_PAGE_SIZE);
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Insert(KeyType key, const ValueType &value, size_t index) {
  if (index >= INTERNAL_PAGE_SIZE)
    return;
  
  MappingType* temp_arr = (MappingType*) malloc(GetMaxSize() - index);
  std::memcpy(temp_arr, &array_[index], GetMaxSize() - index);
    
  if ((int)index != GetSize()) 
    std::memcpy(&array_[index+1], temp_arr, GetMaxSize() - index);

  array_[index] = {key, value};
  SetSize(GetSize() + 1);
}

/**
 * @brief Replaces whatever is at index with key,val pair.
 * 
 * @param key 
 * @param value 
 * @param index 
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::Replace(const KeyType &key, const ValueType &value, size_t index) {
  if (index >= INTERNAL_PAGE_SIZE)
    return;  
  array_[index] = std::pair<KeyType, ValueType>(key,value);
}

/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::KeyAt(int index) const -> KeyType {
  /* + 1 because see B+ Tree Internal Page doc on project #2 webpage */
  return array_[index].first;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::SetKeyAt(int index, const KeyType &key) {
  array_[index].first = key;
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
auto B_PLUS_TREE_INTERNAL_PAGE_TYPE::ValueAt(int index) const -> ValueType {
  return array_[index].second;
}

// valuetype for internalNode should be page id_t
template class BPlusTreeInternalPage<GenericKey<4>, page_id_t, GenericComparator<4>>;
template class BPlusTreeInternalPage<GenericKey<8>, page_id_t, GenericComparator<8>>;
template class BPlusTreeInternalPage<GenericKey<16>, page_id_t, GenericComparator<16>>;
template class BPlusTreeInternalPage<GenericKey<32>, page_id_t, GenericComparator<32>>;
template class BPlusTreeInternalPage<GenericKey<64>, page_id_t, GenericComparator<64>>;
}  // namespace bustub
