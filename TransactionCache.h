#pragma once


using namespace std;

class TransactionCache{
	vector<Transaction_base *> cache;

public:
	TransactionCache();

	void push_back(Transaction_base *ptr);
	void push_front(Transaction_base *ptr);

	Transaction_base *pop_back();
	Transaction_base *pop_front();

	void clear();

};


