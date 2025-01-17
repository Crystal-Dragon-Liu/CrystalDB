//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager_instance.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager_instance.h"
#include "common/util/string_util.h"
#include "common/macros.h"

namespace bustub {
#define CHECK_BUFFER_POOL_UNPINNED \
 if(!CheckBufferPoolUnpinned()){ \
		  return nullptr; \
  } \

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager)
    : BufferPoolManagerInstance(pool_size, 1, 0, disk_manager, log_manager) {}

BufferPoolManagerInstance::BufferPoolManagerInstance(size_t pool_size, uint32_t num_instances, uint32_t instance_index,
                                                     DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size),
      num_instances_(num_instances),
      instance_index_(instance_index),
      next_page_id_(instance_index),
      disk_manager_(disk_manager),
      log_manager_(log_manager) {
  // Instead of pool size, the num_instances should just be 1?
  BUSTUB_ASSERT(num_instances > 0, "If BPI is not part of a pool, then the pool size should just be 1");
  BUSTUB_ASSERT(
      instance_index < num_instances,
      "BPI index cannot be greater than the number of BPIs in the pool. In non-parallel case, index should just be 1.");
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  // TODO implement LRUReplacer.
  //  replacer_ = new LRUReplacer(pool_size);
  replacer_	= new ClockReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManagerInstance::~BufferPoolManagerInstance() {
  delete[] pages_;
  delete replacer_;
}

auto BufferPoolManagerInstance::FlushPgImp(page_id_t page_id) -> bool {
  // Make sure you call DiskManager::WritePage!
  if(page_id == INVALID_PAGE_ID){
		return false;
  }
  std::lock_guard<std::mutex>guard(latch_);
  frame_id_t frame_id;
  if(!SearchPageId(page_id, &frame_id)){ return false;
  }
  if(pages_[frame_id].is_dirty_ == true){
		disk_manager_->WritePage(pages_[frame_id].page_id_, pages_[frame_id].GetData());
		pages_[frame_id].is_dirty_ = false;
  }
  return true;
}

void BufferPoolManagerInstance::FlushAllPgsImp() {
  // You can do it!
  for(size_t i = 0; i < pool_size_; i++){
		  FlushPgImp(pages_[i].page_id_);
  }
}

bool BufferPoolManagerInstance::CheckBufferPoolUnpinned(){
	for(size_t i = 0; i < pool_size_; i++){
			if (this->pages_[i].GetPinCount() == 0) {
					return true;
			}
	}
	return false;
}

bool BufferPoolManagerInstance::GetFrameIdFromFreeList(frame_id_t* frame_id){
	if(this->free_list_.empty()){
			return false;
	}
	else{
			frame_id_t temp = this->free_list_.front();
			*frame_id = temp;
			this->free_list_.remove(temp);
			return true;
	}
}

void BufferPoolManagerInstance::DisplayPageTable(){
		std::lock_guard<std::mutex> guard(latch_);
		PRINT_BLUE("====================");
		PRINT_BLUE("page_id     frame_id");
		for(auto x: page_table_){
				PRINT_BLUE(x.first, "       ", x.second);
		}
		PRINT_BLUE("====================");
}

void BufferPoolManagerInstance::DisplayPagesInfo(){
    std::lock_guard<std::mutex> guard(latch_);
		PRINT_BLUE("==================================");
		PRINT_YELLOW("page_id     frame_id     pin count");
    PRINT_BLUE("==================================");
		for(auto x: page_table_){
				PRINT_YELLOW(x.first, "       ", x.second, "       ", pages_[x.second].pin_count_);
		}
		PRINT_BLUE("==================================");
}

auto BufferPoolManagerInstance::NewPgImp(page_id_t *page_id) -> Page * {
  // 0.   Make sure you call AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  // page_id_t page_id_temp = AllocatePage();
  std::lock_guard<std::mutex>guard(latch_);
  CHECK_BUFFER_POOL_UNPINNED
  *page_id = AllocatePage();
  // Firstly, search page_id_temp from free_list
  frame_id_t frame_id;
  // get frame_id;
  if(!GetFrameIdFromFreeList(&frame_id)){
		  //  PRINT_BLUE("Starting to find a page from replacer....");
		  // PRINT_LOG("Before finding page from replacer");
		  // replacer_->DisplayFrameList();
		  replacer_->Victim(&frame_id);
		 // PRINT_LOG("After finding page from replacer");
		  // replacer_->DisplayFrameList();
		  // delete the <pages_[frame_id].page_id_, frame_id]>
		  page_table_.erase(pages_[frame_id].page_id_); 
		  
  }
  // we return the Pages[frame_id];
  page_table_.insert(std::pair<page_id_t, frame_id_t>(*page_id, frame_id));
  //UPDATE the page metadata and zero out memory.
  //If pages_[frame] is dirty, write this page to disk.
		if(pages_[frame_id].IsDirty()){
			disk_manager_->WritePage(pages_[frame_id].page_id_, pages_[frame_id].GetData());
		}
  pages_[frame_id].ResetMemory();
  pages_[frame_id].page_id_ = *page_id;
  pages_[frame_id].pin_count_ = 1;
  return &pages_[frame_id];
}

auto BufferPoolManagerInstance::FetchPgImp(page_id_t page_id) -> Page * {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::lock_guard<std::mutex> guard(latch_);
  frame_id_t frame_id;
  if(SearchPageId(page_id, &frame_id)){
		replacer_->Pin(frame_id);
		pages_[frame_id].pin_count_++;
		return &pages_[frame_id]; 
  }
  else{
		CHECK_BUFFER_POOL_UNPINNED
		if(!GetFrameIdFromFreeList(&frame_id)){
				replacer_->Victim(&frame_id);
		}
		if(pages_[frame_id].IsDirty()){
				 disk_manager_->WritePage(pages_[frame_id].page_id_, pages_[frame_id].GetData());
		}
		// DELETE <pages_[frame_id].page_id, frame_id> from pages_table_;
		// the pages_[frame_id] which need to be flushed to disk.
		// the new page id is <page_id> which should be recorded to pages_table_.
		page_table_.erase(pages_[frame_id].page_id_); 
		// INSERT <page_id, frame_id> to pages_table_;
		std::pair<page_id_t, frame_id_t> tmp_pair(page_id, frame_id);
		page_table_.insert(tmp_pair);
		// UPDATE P's metadata, read page content from disk.
		pages_[frame_id].page_id_ = page_id;
		pages_[frame_id].pin_count_ = 1;
		disk_manager_->ReadPage(page_id, pages_[frame_id].GetData());
		return &pages_[frame_id];
  }
}

auto BufferPoolManagerInstance::DeletePgImp(page_id_t page_id) -> bool {
  // 0.   Make sure you call DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  std::lock_guard<std::mutex>guard(latch_);
  frame_id_t frame_id;
  if(SearchPageId(page_id, &frame_id)){
	if(pages_[frame_id].pin_count_ != 0){
			return false;
	}
	  // delete page_id from pages_table.
	  page_table_.erase(page_id);
	  // Reset some metadata of pages_[frame_id]
	  pages_[frame_id].ResetMemory();
	  pages_[frame_id].page_id_ = INVALID_PAGE_ID;
	  pages_[frame_id].is_dirty_ = false;
	  // return the page to free_list.
	  free_list_.emplace_back(static_cast<int>(frame_id));
	  DeallocatePage(page_id);
    return true;
  }
  else{
  	return false;
  }
}

auto BufferPoolManagerInstance::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
	
  	std::lock_guard<std::mutex>guard(latch_);
	frame_id_t frame_id;
  	if(SearchPageId(page_id, &frame_id)){
		  if(is_dirty){
		  		pages_[frame_id].is_dirty_ = is_dirty;
	  	  }
		  // adjust the position of unpin.
		  pages_[frame_id].pin_count_--;
		  if(pages_[frame_id].pin_count_ <= 0){
				pages_[frame_id].pin_count_ = 0;
				replacer_->Unpin(frame_id);
		  }
   		  return true;
  }
  else{
    return false;
  }
	
	
}

auto BufferPoolManagerInstance::AllocatePage() -> page_id_t {
  const page_id_t next_page_id = next_page_id_;
  next_page_id_ += num_instances_;
  ValidatePageId(next_page_id);
  return next_page_id;
}

void BufferPoolManagerInstance::ValidatePageId(const page_id_t page_id) const {
  assert(page_id % num_instances_ == instance_index_);  // allocated pages mod back to this BPI
}

}  // namespace bustub
