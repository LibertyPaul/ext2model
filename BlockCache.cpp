#pragma once

#include <new>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include "Chain.h"
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "TransactionCache.h"
#include "error_codes.h"


#include "HDD.h"

#include "BlockCache.h"

	BlockCache::BlockCache(): blockNumber(0), block(nullptr){
	}

	BlockCache::BlockCache(void *block, uint blockNumber=0): blockNumber(blockNumber){
		if(block != nullptr){
			blockSize = hdd->getBlockSize();
			try{
				this->block = operator new(blockSize);
			}catch(bad_alloc &ba){
				cerr<<ba.what()<<endl;
				exit(1);
			}
			memcpy_s(this->block, blockSize, block, blockSize);
		}
		else
			this->block = nullptr;
	}

	BlockCache::BlockCache(const BlockCache &blockCache): blockNumber(blockCache.blockNumber){
		if(blockCache.block != nullptr){
			blockSize = blockCache.blockSize;
			try{
				this->block = operator new(blockSize);
			}catch(bad_alloc &ba){
				cerr<<ba.what()<<endl;
				exit(1);
			}
			memcpy_s(this->block, blockSize, blockCache.block, blockSize);
		}
		else
			this->block = nullptr;
	}

	BlockCache::~BlockCache(){
		if(block != nullptr)
			operator delete(block);
	}

	void *BlockCache::getBlock(){
		if(block == nullptr)
			return nullptr;

		void *blockCopy;
		try{
			blockCopy = operator new(blockSize);
		}catch(bad_alloc &ba){
				cerr<<ba.what()<<endl;
				exit(1);
		}

		memcpy_s(blockCopy, blockSize, this->block, blockSize);

		return blockCopy;
	}

	void BlockCache::rewriteBlock(void *block, uint blockSize){
		memcpy_s(this->block, blockSize, block, blockSize);
	}

	uint BlockCache::getBlockNumber(){
		return blockNumber;
	}

	uint BlockCache::getBlockSize(){
		return blockSize;
	}