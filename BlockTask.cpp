#pragma once

#include <exception>
#include "BlockTask.h"

uint BlockTask::blockSize;

BlockTask::BlockTask(uint blockNumber, uint length, uint start, bool overflow): blockNumber(blockNumber), length(length), start(start), overflow(overflow){
		if(length + start > blockSize)
			throw exception("BlockTask::BlockTask:: block overflow");
	}

	uint BlockTask::getBlockNumber(){
		return blockNumber;
	}

	uint BlockTask::getStartPoint(){
		return start;
	}

	uint BlockTask::getLength(){
		return length;
	}

	void BlockTask::setOverflow(bool overflow){
		this->overflow = overflow;
	}

	bool BlockTask::isOverflowing(){
		return overflow;
	}