#pragma once
#include <exception>
#include "TransactionType.h"

#include "Transaction_base.h"

	Transaction_base::Transaction_base(){
	}


	Transaction_base::Transaction_base(TransactionType type, uint blockNumber): type(type), blockNumber(blockNumber){
	}

	Transaction_base::~Transaction_base(){
	}

	Transaction_base *Transaction_base::clone(){
		return new Transaction_base(*this);
	}

	TransactionType Transaction_base::getType(){
		return type;
	}

	uint Transaction_base::getBlockNumber(){
		return blockNumber;
	}

	void Transaction_base::copyBlock(void *dst){
		throw exception("trying to get info from base transaction");
	}

	void Transaction_base::saveBlock(void *src){
		throw exception("trying to save block in base transaction");
	}

	void Transaction_base::setApplyed(){
		applyed = true;
	}

	bool Transaction_base::isApplyed(){
		return applyed;
	}
