//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// clock_replacer.cpp
//
// Identification: src/buffer/clock_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/clock_replacer.h"

namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) {
		// The ClockReplacer is intialized to have no frame in it.
		this->frame_list_		=	std::vector<frame_id_t>(num_pages, -1);
		this->pin_counts_	=	std::vector<size_t>(num_pages, 0);
		clock_hand_ = 0; // clock_hand_ is set to 0 by default.
}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool {
		frame_id_t temp_hand = clock_hand_;
		do{
				// if there is a frame 
				// with ref_flag which is set to false
				// in the ClockReplacer
				// return the frame_id 
				if(this->frame_list_[temp_hand] != -1)
						if(this->pin_counts[temp_hand] == 0){
								clock_hand_ = temp_hand;
								*frame_id = frame_list_[clock_hand];
								// remove this frame
								frame_list_[clock_hand] = -1;
						}
						// if a frame's ref_flag is set to true.
						else{
								this->pin_counts_[temp_hand]--;
								temp_hand = (++temp_hand) % frame_list_.size();
						}
								
		}while(temp_hand == clock_hand_);
		return false; 
}

void ClockReplacer::Pin(frame_id_t frame_id) {}

void ClockReplacer::Unpin(frame_id_t frame_id) {}

auto ClockReplacer::Size() -> size_t { return 0; }

}  // namespace bustub
