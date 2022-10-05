//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.h
//
// Identification: src/include/buffer/clock_replacer.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <mutex>  // NOLINT
#include <vector>

#include "buffer/replacer.h"
#include "common/config.h"

namespace bustub {

/**
 * ClockReplacer implements the clock replacement policy, which approximates the Least Recently Used policy.
 */
class ClockReplacer : public Replacer {
 public:
  /**
   * Create a new ClockReplacer.
   * @param num_pages the maximum number of pages the ClockReplacer will be required to store
   */
  explicit ClockReplacer(size_t num_pages);

  /**
   * Destroys the ClockReplacer.
   */
  ~ClockReplacer() override;

  /**
   * @brief	Traverse the frame_list_ circularly by the clock hand (clock_hand_). if there is a page in frame_list_ with ref_flag which is set to false, assign the frame_id pointer through the frame_id of this page.  
   * or if the ref_flag of this page in frame_list_ is set to true, reverse it to False. 
   */
  auto Victim(frame_id_t *frame_id) -> bool override;


  /**
   * @brief this method should be called when a page with frame_id is pinned by system,
   * 		this page would be removed in ClockReplacer.
   *
   */
  void Pin(frame_id_t frame_id) override;

  /**
   * @brief	this method should be called when a page with frame_id is unpinned by system,
   * 		this page would be added in ClockReplacer.
   * 		in this implementation, the Unpin method only traverse the frame list once.
   *
   */
  void Unpin(frame_id_t frame_id) override;

  /**
   * @brief check if there is the same frame_id with the unpinned page id.
   * 		ATTENTION this method must be called in thread-safe member function.
   */
  bool CheckFrameId(frame_id_t frame_id);

  auto Size() -> size_t override;

  virtual void DisplayFrameList() override;

  void DisplayClockHand();
 
 private:
  /**
   * @brief check if there is the same frame_id with the unpinned page id.
   * 		ATTENTION this method must be called in thread-safe member functions.
   */
  void UpdateClockHand(frame_id_t temp_hand);

 private:
  //	TODO(student): implement me!
  std::vector<frame_id_t>	frame_list_;
  std::vector<bool>			ref_flags_;
  //	clock hand
  frame_id_t 				clock_hand_;
  //	mutex for all the operations.
  std::mutex				mutex_;  
};

}  // namespace bustub
