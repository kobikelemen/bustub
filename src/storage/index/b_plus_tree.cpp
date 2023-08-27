#include <sstream>
#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
// #include "storage/index/b_plus_tree.h"
#include "storage/index/b_plus_tree.h"


namespace bustub {


/* Returns first index of key where next index is greater than param key.
   Returns index after size_ if param key is bigger than all in page. */
template <typename BPlusTreePageType, typename KeyType, typename KeyComparator>
int FindKeyIndexBefore(BPlusTreePageType *page, KeyType key, KeyComparator comparator) {
  int i = 0;
  for (; i < page->GetSize()-1; i++) {
    if (comparator(page->KeyAt(i + 1), key) == 1) 
      return i;
  }
  return i + 1;
}


/* Returns index to insert key so it's inserted in sorted way */
template <typename BPlusTreePageType, typename KeyType, typename KeyComparator>
int FindKeyIndexBeforeLeaf(BPlusTreePageType *page, KeyType key, KeyComparator comparator) {
  if (page->GetSize() == 0)
    return 0;
  int i = 0;
  for (; i < page->GetSize(); i++) {
    /* If KeyAt(i) >= key ... */
    if (comparator(page->KeyAt(i), key) == 1 || 
        comparator(page->KeyAt(i), key) == 1) {
        return i;  
      }
  }
  return i;
}


template <typename BPlusTreePageType, typename KeyType, typename KeyComparator>
int FindGuidepostIndexInternal(BPlusTreePageType *page, KeyType key, KeyComparator comparator) {
  /* If first guidepost is greater than key looking for, return first key (invalid key) */
  if (comparator(page->KeyAt(1), key) == 1) {
    return 0;
  }
  /* Return first guidepost index where next guidepost is greater than key. 
     Return last index if no others satisfy. */
  int i = 1;
  for (; i < page->GetSize()-1; i++) {
    if (comparator(page->KeyAt(i + 1), key) == 1) 
      return i;
  }
  return i;
}


/* Returns index of key that matches param key if in page.
   Returns -1 if key not in page. */
template <typename BPlusTreePageType, typename KeyType, typename KeyComparator>
int FindKey(BPlusTreePageType *page, KeyType key, KeyComparator comparator) {
  int i = 0;
  for (; i < page->GetSize(); i++) {
    if (comparator(page->KeyAt(i), key) == 0) 
      return i;
  }
  return -1;
}



INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, page_id_t header_page_id, BufferPoolManager *buffer_pool_manager,
                          const KeyComparator &comparator, int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      bpm_(buffer_pool_manager),
      comparator_(std::move(comparator)),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size),
      header_page_id_(header_page_id) {
  // WritePageGuard guard = bpm_->FetchPageWrite(header_page_id_);
  // auto root_page = guard.AsMut<BPlusTreeHeaderPage>();
  // root_page->root_page_id_ = INVALID_PAGE_ID;
  INVALID_KEY.SetFromInteger(-69);
  auto header_page = reinterpret_cast<BPlusTreeHeaderPage*>(bpm_->FetchPage(header_page_id)->GetData());
  bpm_->NewPage(&header_page->root_page_id_);
  LeafPage* root_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(header_page->root_page_id_)->GetData());
  root_page->Init();
  bpm_->UnpinPage(header_page->root_page_id_, true);
  bpm_->UnpinPage(header_page_id, true);
  size_ = 0;
}


INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetMaxLeafPageId() -> page_id_t {
  page_id_t current_page_id = GetRootPageId();
  auto page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(current_page_id)->GetData());
  while (!page->IsLeafPage()) {
    int index = page->GetSize() - 1;
    InternalPage *prev_page = page;
    current_page_id = prev_page->ValueAt(index);
    page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(current_page_id)->GetData());
    bpm_->UnpinPage(current_page_id, false);
  }
  return current_page_id;
}


INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetLeafPageId(KeyType key) -> page_id_t {
  auto page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(GetRootPageId())->GetData());
  page_id_t current_page_id;

  while (!page->IsLeafPage()) {
    int index = FindGuidepostIndexInternal<InternalPage,KeyType,KeyComparator>(page, key, comparator_);
    InternalPage *prev_page = page;
    current_page_id = prev_page->ValueAt(index);
    page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(current_page_id)->GetData());
    bpm_->UnpinPage(current_page_id, false);
  }
  return current_page_id;
}



INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetLeafPage(KeyType key, std::vector<page_id_t> &page_path) -> page_id_t {
  /* Start searching at root page */
  auto page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(GetRootPageId())->GetData());
  page_path.push_back(GetRootPageId());
  while (!page->IsLeafPage()) {
    int index = FindGuidepostIndexInternal<InternalPage,KeyType,KeyComparator>(page, key, comparator_);
    InternalPage *prev_page = page;
    page_id_t current_page_id = prev_page->ValueAt(index);
    page_path.push_back(current_page_id);
    page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(current_page_id)->GetData());
    bpm_->UnpinPage(current_page_id, false);
  }
  page_id_t leaf_page_id = page_path.back();
  page_path.pop_back();
  return leaf_page_id;
}



/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() -> bool {
  auto root_page_id = GetRootPageId();
  auto root_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(root_page_id)->GetData());
  if (root_page->GetSize() > 0) {
    return false;
  }
  return true;
}



/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *txn) -> bool {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;
  std::vector<page_id_t> page_path;
  auto leaf_page_id = GetLeafPage(key, page_path);
  auto leaf_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(leaf_page_id)->GetData());
  int index = FindKey<LeafPage,KeyType,KeyComparator>(leaf_page, key, comparator_);
  if (index != -1) {
    *result = {leaf_page->ValueAt(index)};
    return true;
  }
  *result = {};
  return false;
}



/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *txn) -> bool {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;
  
  /* Saved order in which pages were traversed to reach leaf_page.
     Does not include leaf_page. (last one is parent of it.) */
  std::vector<page_id_t> page_path;
  auto leaf_page_id = GetLeafPage(key, page_path);
  auto leaf_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(leaf_page_id)->GetData());

  if (leaf_page->GetSize() < leaf_page->GetMaxSize()) { /* If not full */
    /* Check if duplicate, return false if it is, insert and return true if not. */
    int index = FindKeyIndexBeforeLeaf<LeafPage,KeyType,KeyComparator>(leaf_page, key, comparator_);
    if (index > 0 && comparator_(leaf_page->KeyAt(index-1), key) == 0) 
      return false;
    leaf_page->Insert(key, value, index);
  } else {
    /* Split leaf page (TODO: make split recursive) */
    std::tuple<page_id_t,page_id_t,KeyType> leaf_page_tuple = SplitLeafNode(leaf_page_id);
    page_id_t left_page_id = std::get<0>(leaf_page_tuple);
    page_id_t right_page_id = std::get<1>(leaf_page_tuple);
    KeyType key_middle = std::get<2>(leaf_page_tuple);
    LeafPage* left_page_ptr = reinterpret_cast<LeafPage*>(bpm_->FetchPage(left_page_id)->GetData());
    LeafPage* right_page_ptr = reinterpret_cast<LeafPage*>(bpm_->FetchPage(right_page_id)->GetData());

    UpdateParent(left_page_id, right_page_id, key_middle, page_path);
    left_page_ptr->SetNextPageId(right_page_id);
    right_page_ptr->SetNextPageId(leaf_page->GetNextPageId());

    KeyType left_page_end_key = left_page_ptr->KeyAt(left_page_ptr->GetSize()-1);
    LeafPage* target_page;
    /* If end key in left page is smaller or equal to key. */
    if (comparator_(left_page_end_key, key) == 1 || comparator_(left_page_end_key, key) == 0) {
      target_page = left_page_ptr;
    } else {
      target_page = right_page_ptr;
    }
    size_t index = (size_t)FindKeyIndexBeforeLeaf<LeafPage,KeyType,KeyComparator>(target_page, key, comparator_);
    target_page->Insert(key, value, index);
    // TODO unpin left_page_ptr and right_page_ptr
  }
  size_ ++;
  return true;
  /*
   - Look for biggest key that is less than or equal to key searching for 
   - Follow pointer to level below and repeat recursively until at leaf layer
   - Search for kbiggest key that is less than or equal to insert key and insert
     key-value at that location
   - if leaf layer full then need to split current ndoe into two and move
     middle value to layer above. The value moved should point to newly made node. 
     Repeat this recursively if layer above is also filled.

  */
}




INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::UpdateParent(page_id_t left_page_id, page_id_t right_page_id,
                  KeyType key_middle, std::vector<page_id_t> page_path) -> void {
  page_id_t parent_page_id;
  if (page_path.size() > 0) { /* Add middle elem to parent page */
      parent_page_id = page_path.back();
      page_path.pop_back();
      auto parent_page = reinterpret_cast<InternalPage*>(bpm_->FetchPage(parent_page_id)->GetData());
      parent_page->SetPageType(IndexPageType::INTERNAL_PAGE);
      int index = FindGuidepostIndexInternal<InternalPage,KeyType,KeyComparator>(parent_page, key_middle, comparator_);
      /* Old guidepost points to left_page_ptr now and key_middle guidepost points to right_page_ptr */
      parent_page->Replace(parent_page->KeyAt(index), left_page_id, index);
      parent_page->Insert(key_middle, right_page_id, index+1);

    } else { /* Create new page that is now parent page and add middle elem */
      InternalPage* parent_page = reinterpret_cast<InternalPage*>(bpm_->NewPage(&parent_page_id)->GetData());
      parent_page->Init();
      /* Initialise invalid key ptr to point to left_page_ptr and first guidepost to point to right_page_ptr */
      parent_page->Insert(key_middle, left_page_id, 0);
      parent_page->Insert(key_middle, right_page_id, 1);
      SetRootPageId(parent_page_id);
    }
    bpm_->UnpinPage(parent_page_id, true);
}



/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *txn) {
  // Declaration of context instance.
  Context ctx;
  (void)ctx;
  size_ --;
  /* IMPORTANT: Insert() relies on compacting leaf array when removing an element. */
  /* IMPORTANT: Ensure next_page_id_ is updated including to -1 to prev leaf page 
                if removed leaf page is end one. */
}



INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::SplitLeafNode(page_id_t leaf_page_id) -> std::tuple<page_id_t,page_id_t,KeyType> {
  auto leaf_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(leaf_page_id)->GetData());
  int index_middle = (int)leaf_page->GetSize() / 2;
  KeyType key_middle = leaf_page->KeyAt(index_middle);
  page_id_t right_page_id;
  auto right_page_ptr = reinterpret_cast<LeafPage*>(bpm_->NewPage(&right_page_id)->GetData());
  right_page_ptr->Init();
  for (int i=0; i < (int)leaf_page->GetSize(); i ++) {
    
    if (i >= index_middle) {
      right_page_ptr->Insert(leaf_page->KeyAt(i), leaf_page->ValueAt(i), right_page_ptr->GetSize());
    }
  }
  leaf_page->SetSize(index_middle - 1);
  bpm_->UnpinPage(right_page_id, true);
  return {leaf_page_id, right_page_id, key_middle};
}



/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE {
  KeyType index_key;
  index_key.SetFromInteger(0);
  return {bpm_, {GetLeafPageId(index_key), 0}};
}



/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE { 
  std::vector<page_id_t> page_path;
  auto leaf_page_id = GetLeafPage(key, page_path);
  auto leaf_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(leaf_page_id)->GetData());
  for (int offset=0; offset < leaf_page->GetSize(); offset++) {
    if (comparator_(leaf_page->KeyAt(offset), key) == 0) {
      return {bpm_, {leaf_page_id, offset}};
    }
  }
  return INDEXITERATOR_TYPE(); 
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { 
  page_id_t max_leaf_page_id = GetMaxLeafPageId();
  LeafPage* max_leaf_page = reinterpret_cast<LeafPage*>(bpm_->FetchPage(max_leaf_page_id)->GetData());
  return {bpm_, {max_leaf_page_id, max_leaf_page->GetSize()}};
}



/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t {
  auto header_page = reinterpret_cast<BPlusTreeHeaderPage*>(bpm_->FetchPage(header_page_id_)->GetData());
  page_id_t root_page_id = header_page->root_page_id_;
  bpm_->UnpinPage(header_page_id_, false);
  return root_page_id;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::SetRootPageId(page_id_t root_page_id) {
  auto header_page = reinterpret_cast<BPlusTreeHeaderPage*>(bpm_->FetchPage(header_page_id_)->GetData());
  header_page->root_page_id_ = root_page_id;
  bpm_->UnpinPage(header_page_id_, true);
}


/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, txn);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *txn) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, txn);
  }
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  auto root_page_id = GetRootPageId();
  // auto guard = bpm->FetchPageBasic(root_page_id);
  auto root_page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(root_page_id)->GetData());
  PrintTree(root_page_id, root_page);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::PrintTree(page_id_t page_id, const BPlusTreePage *page) {
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    std::cout << "Leaf Page: " << page_id << "\tNext: " << leaf->GetNextPageId() << std::endl;

    // Print the contents of the leaf page.
    std::cout << "Contents: ";
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i);
      if ((i + 1) < leaf->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
    std::cout << std::endl;

  } else {
    auto *internal = reinterpret_cast<const InternalPage *>(page);
    std::cout << "Internal Page: " << page_id << std::endl;

    // Print the contents of the internal page.
    std::cout << "Contents: ";
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i);
      if ((i + 1) < internal->GetSize()) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      // auto guard = bpm_->FetchPageBasic(internal->ValueAt(i));
      auto* page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(internal->ValueAt(i))->GetData());
      PrintTree(internal->ValueAt(i), page);
    }
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Drawing an empty tree");
    return;
  }

  std::ofstream out(outf);
  out << "digraph G {" << std::endl;
  auto root_page_id = GetRootPageId();
  // auto guard = bpm->FetchPageBasic(root_page_id);
  auto* root_page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(root_page_id)->GetData());
  ToGraph(root_page_id, root_page, out);
  out << "}" << std::endl;
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(page_id_t page_id, const BPlusTreePage *page, std::ofstream &out) {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    auto *leaf = reinterpret_cast<const LeafPage *>(page);
    // Print node name
    out << leaf_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << page_id << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << page_id << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }
  } else {
    auto *inner = reinterpret_cast<const InternalPage *>(page);
    // Print node name
    out << internal_prefix << page_id;
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << page_id << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      // auto child_guard = bpm_->FetchPageBasic(inner->ValueAt(i));
      page_id_t child_page_id = inner->ValueAt(i);
      // auto child_page = child_guard.template As<BPlusTreePage>();
      auto* child_page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(inner->ValueAt(i), child_page, out);
      if (i > 0) {
        // auto sibling_guard = bpm_->FetchPageBasic(inner->ValueAt(i - 1));
        // auto sibling_page = sibling_guard.template As<BPlusTreePage>();
        auto sibling_page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(inner->ValueAt(i - 1))->GetData());
        page_id_t sibling_page_id = inner->ValueAt(i - 1);
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page_id << " " << internal_prefix
              << child_page_id << "};\n";
        }
      }
      out << internal_prefix << page_id << ":p" << child_page_id << " -> ";
      if (child_page->IsLeafPage()) {
        out << leaf_prefix << child_page_id << ";\n";
      } else {
        out << internal_prefix << child_page_id << ";\n";
      }
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::DrawBPlusTree() -> std::string {
  if (IsEmpty()) {
    return "()";
  }

  PrintableBPlusTree p_root = ToPrintableBPlusTree(GetRootPageId());
  std::ostringstream out_buf;
  p_root.Print(out_buf);

  return out_buf.str();
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::ToPrintableBPlusTree(page_id_t root_id) -> PrintableBPlusTree {
  // auto root_page_guard = bpm_->FetchPageBasic(root_id);
  // auto root_page = root_page_guard.template As<BPlusTreePage>();
  auto root_page = reinterpret_cast<BPlusTreePage*>(bpm_->FetchPage(root_id)->GetData());
  PrintableBPlusTree proot;

  if (root_page->IsLeafPage()) {
    // auto leaf_page = root_page_guard.template As<LeafPage>();
    auto leaf_page = reinterpret_cast<LeafPage*>(root_page);
    proot.keys_ = leaf_page->ToString();
    proot.size_ = proot.keys_.size() + 4;  // 4 more spaces for indent

    return proot;
  }

  // draw internal page
  // auto internal_page = root_page_guard.template As<InternalPage>();
  auto internal_page = reinterpret_cast<InternalPage*>(root_page);
  proot.keys_ = internal_page->ToString();
  proot.size_ = 0;
  for (int i = 0; i < internal_page->GetSize(); i++) {
    page_id_t child_id = internal_page->ValueAt(i);
    PrintableBPlusTree child_node = ToPrintableBPlusTree(child_id);
    proot.size_ += child_node.size_;
    proot.children_.push_back(child_node);
  }

  return proot;
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;

template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;

template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;

template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;

template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
