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
    LeafPage* page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(pos_.first)->GetData());
    if (page->GetNextPageId() == -1 && (int)pos_.second == page->GetSize()-1) {
        return true;
    }
    return false;
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator*() -> const MappingType & {
    LeafPage* page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(pos_.first)->GetData());
    return page->KeyValueAt((int)pos_.second);

}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator++() -> INDEXITERATOR_TYPE & {
    LeafPage* page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(pos_.first)->GetData());
    if ((int)pos_.second >= page->GetSize()) {
        pos_ = {page->GetNextPageId(), 0};
    } else {
        pos_.second ++;
    }
    return *this;
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator==(const IndexIterator &itr) const -> bool {
    if (pos_.first == itr.GetPos().first && pos_.second == itr.GetPos().second) {
        return true;
    }
    return false;
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator!=(const IndexIterator &itr) const -> bool {
    if (pos_.first == itr.GetPos().first && pos_.second == itr.GetPos().second) {
        return false;
    }
    return true;
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::GetPos() const -> std::pair<page_id_t, size_t> {
    return pos_;
}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
