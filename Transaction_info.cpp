#pragma once
#include <memory.h>
#include "TransactionType.h"
#include "Transaction_base.h"

#include "Transaction_info.h"

	Transaction_info::Transaction_info(TransactionType type, uint blockNumber, void *block){
		this->type = type;
		this->blockNumber = blockNumber;

		this->block = operator new(blockSize);
		memcpy_s(this->block, blockSize, block, blockSize);
	}

	Transaction_info::Transaction_info(const Transaction_info &tr){
		this->block = operator new(blockSize);
		memcpy_s(this->block, blockSize, tr.block, blockSize);
		this->type = tr.type;
		this->blockNumber = tr.blockNumber;
		this->applyed = tr.applyed;
	}

	Transaction_info::~Transaction_info(){
		operator delete(block);
	}

	void Transaction_info::copyBlock(void *dst){
		memcpy_s(dst, blockSize, block, blockSize);
	}

	void Transaction_info::saveBlock(void *src){
		memcpy_s(this->block, blockSize, src, blockSize);
	}

	Transaction_info *Transaction_info::clone(){
		return new Transaction_info(*this);
	}