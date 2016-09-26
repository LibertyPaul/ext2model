#pragma once
#include <vector>
using namespace std;
typedef unsigned int uint;

class Chain{
	uint start;
	uint length;
public:
	Chain(uint start=0, uint length=0):start(start), length(length){
	}

	Chain(const Chain &T){
		start=T.start;
		length=T.length;
	}

	uint decLength(){
		return --length;
	}

	uint getFrontNumber(){
		return start;
	}

	uint getLength(){
		return length;
	}

	uint getBackNumber(){
		return start+length-1;
	}

	void setEndOfChunk(uint next){
		start=0;
		length=next;
	}

	void appendBlock(uint blockNumber){
		if(blockNumber == getFrontNumber() - 1)--start;
		if(blockNumber == getBackNumber() + 1)++length;
	}

	void appendChain(Chain chain){
		if(this->getBackNumber() + 1 == chain.getFrontNumber())
			length += chain.getLength();
		if(chain.getBackNumber() + 1 == this->getFrontNumber()){
			length += chain.getLength();
			this->start -= chain.getLength();
		}
	}

	void drop(){
		length=0;
		start=0;
	}

	bool operator > (Chain T){//для сортировки массива
		return this->start > T.start;
	}

	bool operator < (Chain T){
		return this->start < T.start;
	}

	Chain operator=(Chain T){
		start=T.start;
		length=T.length;
		return *this;
	}

	void setStart(uint T){
		start=T;
		length=1;
	}

	bool isInRange(uint blockNumber){
		return (blockNumber >= getFrontNumber() && blockNumber <= getBackNumber());
		}

	vector<uint> getList(){
		vector<uint> result;
		for(uint i=getFrontNumber(); i<=getBackNumber(); ++i)
			result.push_back(i);
		return result;
	}

};