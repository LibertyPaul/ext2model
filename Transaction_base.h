#pragma once

typedef unsigned int uint;
using namespace std;

class Transaction_base{
protected:
	bool applyed;
	TransactionType type;
	uint blockNumber;
	Transaction_base();

public:
	virtual ~Transaction_base();
	static uint blockSize;
	Transaction_base(TransactionType type, uint blockNumber);
	virtual Transaction_base *clone();
	TransactionType getType();
	uint getBlockNumber();
	virtual void copyBlock(void *dst);
	virtual void saveBlock(void *src);
	void setApplyed();
	bool isApplyed();
};