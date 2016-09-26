#pragma once

typedef unsigned int uint;

class BlockCache{//кэш одного блока
	uint blockNumber;
	uint blockSize;
	void *block;

public:
	static HDD *hdd;
	BlockCache();
	BlockCache(void *block, uint blockNumber);
	BlockCache(const BlockCache &blockCache);
	~BlockCache();
	void *getBlock();
	void rewriteBlock(void *block, uint blockSize);
	uint getBlockNumber();
	uint getBlockSize();
};
