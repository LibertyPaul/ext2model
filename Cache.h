#pragma once

using namespace std;
typedef unsigned int uint;

class Cache{
	static const uint cacheSize = 50;
	vector<BlockCache> blockCache;

public:
	static HDD *hdd;

	void *readBlock(uint blockNumber);
	void writeBlock(uint blockNumber, void *block);
	void setInvalid(uint blockNumber);
};
