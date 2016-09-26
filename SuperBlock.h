#pragma once
typedef unsigned int uint;

struct SuperBlock{
	uint magicNumber;
	uint blockSize;
	uint blockCount;
	uint freeBlockCount;
	LocationInfo storageBackup;
};