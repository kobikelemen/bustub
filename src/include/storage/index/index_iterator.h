//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/include/index/index_iterator.h
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once

// #include "storage/index/b_plus_tree.h"
#include "storage/page/b_plus_tree_leaf_page.h"

namespace bustub {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

// INDEX_TEMPLATE_ARGUMENTS class IndexIterator;




INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {

  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

 public:
  // you may define your own constructor based on your member variables
  IndexIterator();
  IndexIterator(BufferPoolManager* bpm, std::pair<page_id_t,size_t> pos);
  ~IndexIterator();  // NOLINT

  auto IsEnd() -> bool;

  auto operator*() -> const MappingType &;

  auto operator++() -> IndexIterator &;

  auto operator==(const IndexIterator &itr) const -> bool;

  auto operator!=(const IndexIterator &itr) const -> bool;

  auto GetPos() const -> std::pair<page_id_t, size_t>;


 private:
  
  // size_t pos_;

  size_t start_;

  size_t end_;

  std::pair<page_id_t,size_t> pos_;

  BufferPoolManager* bpm_;


};

}  // namespace bustub
