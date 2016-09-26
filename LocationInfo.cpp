#pragma once

#include <exception>


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
#include "FifteenFields.h"

#include "LocationInfo.h"



	LocationInfo::LocationInfo(FifteenFields placement, luint residueSize): placement(placement), residueSize(residueSize){
		blockCount = placement.fileSize();
	}

	LocationInfo::LocationInfo(){
		blockCount = 0;
		residueSize = 0;
	}


	uint LocationInfo::resolve15fields(uint blockNumber){
		if(blockNumber >= blockCount)
			throw std::exception("File isn't too large");
		uint result;
		try{
			result = placement.resolveFifteenFields(blockNumber);
		}catch(std::exception &exc){
			std::cerr<<exc.what()<<std::endl;
		}
		return result;
	}

	uint LocationInfo::appendField(){
		hdd->takeBlocks(1);
		hdd->applyTransactions();
		uint newField = hdd->popBlockStack();
		if(newField == 0)
			return 0;
		placement.editField(blockCount, newField);
		++blockCount;
		residueSize += hdd->getBlockSize();
		return newField;
	}

	uint LocationInfo::getBlockCount(){
		return blockCount;
	}

	luint LocationInfo::getResidueSize(){
		return residueSize;
	}

	luint LocationInfo::getRealFileSize(){
		return hdd->getBlockSize() * blockCount - residueSize;
	}

	int LocationInfo::freeBlocks(){
		vector<uint> blocks;
		for(uint i = 0; i < blockCount; ++i){
			blocks.push_back(placement.resolveFifteenFields(i));
			placement.editField(i, 0);
		}
		if(blocks.empty())
			return 0;
		if(hdd->freeBlocks(blocks) == -1)
			throw exception("hdd error #" + hdd->getLastError().getInternalCode());

		return 0;
	}

	int LocationInfo::resize(luint size){
		long long int current = size;
		current -= getResidueSize();
		uint blockSize = hdd->getBlockSize();

		while(current > 0){
			if(appendField() == 0)
				return -1;
			current -= blockSize;
		}
		residueSize = - current;

		return 0;
	}
