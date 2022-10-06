//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// parallel_buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/parallel_buffer_pool_manager.h"
#include "buffer/buffer_pool_manager_instance.h"
#include "common/util/string_util.h"
namespace bustub {

#define GET_RESPONSIBLE_INDEX page_id % num_instances_

ParallelBufferPoolManager::ParallelBufferPoolManager(size_t num_instances, size_t pool_size, DiskManager *disk_manager,
                                                     LogManager *log_manager){
  //TODO Allocate and create individual BufferPoolManagerInstances
  this->num_instances_ = num_instances;
  this->start_index_ = 0;
  this->bpmi_vec_ = new BufferPoolManagerInstance*[num_instances];
  for(size_t i = 0; i < num_instances; ++i){
    this->bpmi_vec_[i] = new BufferPoolManagerInstance(pool_size, num_instances, i, disk_manager, log_manager);
  }

}

// Update constructor to destruct all BufferPoolManagerInstances and deallocate any associated memory
ParallelBufferPoolManager::~ParallelBufferPoolManager(){
  for(size_t i = 0; i < this->num_instances_; i++){
    delete this->bpmi_vec_[i];
    this->bpmi_vec_[i] = nullptr;
  }
  delete [] this->bpmi_vec_; 
}

auto ParallelBufferPoolManager::GetPoolSize() -> size_t {
  // Get size of all BufferPoolManagerInstances
  return this->start_index_;
}

auto ParallelBufferPoolManager::GetBufferPoolManager(page_id_t page_id) -> BufferPoolManager * {
  // Get BufferPoolManager responsible for handling given page id. You can use this method in your other methods.
  size_t instances_index = GET_RESPONSIBLE_INDEX;
  return this->bpmi_vec_[instances_index];
}

auto ParallelBufferPoolManager::FetchPgImp(page_id_t page_id) -> Page * {
  // Fetch page for page_id from responsible BufferPoolManagerInstance
  size_t instances_index = GET_RESPONSIBLE_INDEX;
  return this->bpmi_vec_[instances_index]->FetchPage(page_id);
}

auto ParallelBufferPoolManager::UnpinPgImp(page_id_t page_id, bool is_dirty) -> bool {
  // Unpin page_id from responsible BufferPoolManagerInstance
  size_t instances_index = GET_RESPONSIBLE_INDEX;
  return this->bpmi_vec_[instances_index]->UnpinPage(page_id, is_dirty);
}

auto ParallelBufferPoolManager::FlushPgImp(page_id_t page_id) -> bool {
  // Flush page_id from responsible BufferPoolManagerInstance
  size_t instances_index = GET_RESPONSIBLE_INDEX;
  return this->bpmi_vec_[instances_index]->FlushPage(page_id);
}

auto ParallelBufferPoolManager::NewPgImp(page_id_t *page_id) -> Page * {
  // create new page. We will request page allocation in a round robin manner from the underlying
  // BufferPoolManagerInstances
  // 1.   From a starting index of the BPMIs, call NewPageImpl until either 1) success and return 2) looped around to
  // starting index and return nullptr
  // 2.   Bump the starting index (mod number of instances) to start search at a different BPMI each time this function
  // is called
  size_t temp_ins_index = start_index_;
  do{
    auto page = this->bpmi_vec_[temp_ins_index]->NewPage(page_id);
    if(page != nullptr){
	  UpdateStartingIndex(temp_ins_index);
      return page;
    }
    temp_ins_index = (++temp_ins_index) % this->num_instances_;

  }while (temp_ins_index != this->start_index_);
  return nullptr;
}

auto ParallelBufferPoolManager::UpdateStartingIndex(size_t temp_index) -> void {
		this->start_index_ = (++temp_index) % this->num_instances_;
}

auto ParallelBufferPoolManager::DeletePgImp(page_id_t page_id) -> bool {
  // Delete page_id from responsible BufferPoolManagerInstance
  size_t instances_index = GET_RESPONSIBLE_INDEX;
  return this->bpmi_vec_[instances_index]->DeletePage(page_id);
}

void ParallelBufferPoolManager::FlushAllPgsImp() {
  // flush all pages from all BufferPoolManagerInstances
  for(size_t i = 0; i < this->num_instances_; i++){
    this->bpmi_vec_[i]->FlushAllPages();
  }
}

auto ParallelBufferPoolManager::DisplayAllPagesTable() -> void{
  for(size_t i = 0; i < this->num_instances_; i++){
    PRINT_RED("===============", i,"===============");
    this->bpmi_vec_[i]->DisplayPagesInfo();
  }
}

}  // namespace bustub
