#pragma once

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

#include "Cache.h"

void *Cache::readBlock(uint blockNumber){
		for(auto bc : blockCache)
			if(bc.getBlockNumber() == blockNumber)
				return bc.getBlock();

		//блок в кэше не найден.

		if(blockCache.size() == cacheSize)//если кэш полон
			blockCache.pop_back();//удаляем самый старый блок

		hdd->readMemory(blockNumber);
		hdd->applyTransactions();
		void *block = operator new(hdd->getBlockSize());
		hdd->popReadStack(block);

		BlockCache bc(block, blockNumber);
		blockCache.insert(blockCache.begin(), bc);

		return block;
	}

	void Cache::writeBlock(uint blockNumber, void *block){
		hdd->writeMemory(blockNumber, block);
		hdd->applyTransactions();

		for(auto i= blockCache.begin(); i != blockCache.end(); ++i)
			if(i->getBlockNumber() == blockNumber)
				i->rewriteBlock(block, hdd->getBlockSize());
	}

	void Cache::setInvalid(uint blockNumber){
		for(auto i = blockCache.begin(); i != blockCache.end(); ++i)
			if(i->getBlockNumber() == blockNumber)
				blockCache.erase(i);
	}