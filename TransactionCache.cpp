#pragma once

#include <memory.h>
#include <vector>
#include "TransactionType.h"
#include "Transaction_base.h"

#include "TransactionCache.h"


	TransactionCache::TransactionCache(){
	}

	void TransactionCache::push_back(Transaction_base *ptr){
		cache.push_back(ptr->clone());
	}

	void TransactionCache::push_front(Transaction_base *ptr){
		cache.insert(cache.begin(), ptr->clone());
	}

	Transaction_base *TransactionCache::pop_back(){
		if(cache.empty())return nullptr;
		Transaction_base *ptr = cache.back();
		cache.pop_back();
		return ptr;
	}

	Transaction_base *TransactionCache::pop_front(){
		if(cache.empty())return nullptr;
		Transaction_base *ptr = cache.front();
		cache.erase(cache.begin());
		return ptr;
	}

	void TransactionCache::clear(){
		for(auto tr : cache)
			delete tr;
		cache.clear();
	}