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
#include "include/common/util/string_util.h"
namespace bustub {

ClockReplacer::ClockReplacer(size_t num_pages) {
		// The ClockReplacer is intialized to have no frame in it.
		this->frame_list_		=	std::vector<frame_id_t>(num_pages, -1);
		this->ref_flags_		=	std::vector<bool>(num_pages, false);
		this->clock_hand_ = 0; // clock_hand_ is set to 0 by default.
}

ClockReplacer::~ClockReplacer() = default;

auto ClockReplacer::Victim(frame_id_t *frame_id) -> bool {
		// TODO lock member structure.
		std::lock_guard<std::mutex> guard(mutex_);
		frame_id_t temp_hand = this->clock_hand_;
		do{
				// if there is a frame 
				// with ref_flag which is set to false
				// in the ClockReplacer
				// return the frame_id 
				if(this->frame_list_[temp_hand] != -1){
						if(this->ref_flags_[temp_hand] == false){
								*frame_id = this->frame_list_[temp_hand];
								// delete frame with frame_id from frame_list_.
								this->frame_list_[temp_hand] = -1;
								this->ref_flags_[temp_hand] = false;
								// this->clock_hand_ = (++temp_hand) % this->frame_list_.size();
								UpdateClockHand(temp_hand);
								return true;
						}
						// if a frame's ref_flag is set to true.
						else{
								this->ref_flags_[temp_hand] = false;
						}
				}
				temp_hand = (++temp_hand) % this->frame_list_.size();
		}while(1);
		return false; 
}

void ClockReplacer::Pin(frame_id_t frame_id) {
		std::lock_guard<std::mutex> guard(mutex_);
		frame_id_t temp_hand = this->clock_hand_;
		do{
				if(this->frame_list_[temp_hand] == frame_id){
						this->frame_list_[temp_hand] = -1;
						this->ref_flags_[temp_hand] = false;
						UpdateClockHand(temp_hand);
						return;
				}
				temp_hand = (++temp_hand) % this->frame_list_.size();
		}while(temp_hand != this->clock_hand_);
}

void ClockReplacer::Unpin(frame_id_t frame_id) {
		std::lock_guard<std::mutex> guard(mutex_);
		frame_id_t temp_hand = this->clock_hand_;
		if(!CheckFrameId(frame_id))
				return;
		do{
				if(this->frame_list_[temp_hand] == -1){
						this->frame_list_[temp_hand] = frame_id;
						this->ref_flags_[temp_hand] = true;
						UpdateClockHand(temp_hand);
						return;
				}
				temp_hand = (++temp_hand) % this->frame_list_.size();
		}while(temp_hand != this->clock_hand_);
}

bool ClockReplacer::CheckFrameId(frame_id_t frame_id){
		for(size_t i = 0; i < this->frame_list_.size();i++){
				if(this->frame_list_[i] == frame_id) return false;
		}
		return true;
}

void ClockReplacer::UpdateClockHand(frame_id_t temp_hand){
		this->clock_hand_ = (++temp_hand) % this->frame_list_.size();
}

auto ClockReplacer::Size() -> size_t {
		std::lock_guard<std::mutex> guard(mutex_);
		size_t counts = 0;
		for (size_t i = 0; i < this->frame_list_.size(); i++){
				if(this->frame_list_[i] != -1) counts++;
		}
		return counts;
}

void ClockReplacer::DisplayFrameList(){
		std::cout << "[";
		for(size_t i = 0; i < this->frame_list_.size(); i++){
				std::cout << this->frame_list_[i] << " ";
		}
		std::cout << "]" << std::endl;
}

void ClockReplacer::DisplayClockHand(){
		PRINT("clock hand index: ", this->clock_hand_);
}

}
