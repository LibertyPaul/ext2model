#pragma once

typedef unsigned int uint;

class Transaction_info : public Transaction_base{
	void *block;


public:
	Transaction_info(TransactionType type, uint blockNumber, void *block);
	Transaction_info(const Transaction_info &tr);
	virtual ~Transaction_info();
	virtual void copyBlock(void *dst);
	virtual void saveBlock(void *src);
	virtual Transaction_info *clone();
};