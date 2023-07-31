/**
 * index_iterator.cpp
 */
#include <cassert>

#include "storage/index/index_iterator.h"


namespace bustub {

/*
 * NOTE: you can change the destructor/constructor method here
 * set your own input parameters
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator() = default;

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(BufferPoolManager* bpm, std::pair<page_id_t,size_t> pos) {
    // b_plus_tree_ = associated_b_plus_tree;
    bpm_ = bpm;
    pos_ = pos;
}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator() = default;  // NOLINT

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::IsEnd() -> bool {
    return pos_ == b_plus_tree_.size_;
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator*() -> const MappingType & {
    LeafPage* page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(pos_.first)->GetData());
    return page->array_[pos_.second];

}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator++() -> INDEXITERATOR_TYPE & {
    LeafPage* page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(pos_.first)->GetData());
    if (pos_.second >= page->GetSize()) {
        pos_ = {page->GetNextPageId(), 0};
    } else {
        pos_.second ++;
    }
}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
