#pragma once

typedef unsigned int uint;
typedef long unsigned int luint;

class LocationInfo{
	FifteenFields placement;
	uint blockCount;
	luint residueSize;//размер пустого места в хвосте

public:
	static HDD *hdd;
	
	LocationInfo(FifteenFields placement, luint residueSize);
	LocationInfo();
	uint resolve15fields(uint blockNumber);
	uint appendField();
	uint getBlockCount();
	luint getResidueSize();
	luint getRealFileSize();
	int resize(luint size);
	int freeBlocks();//освобождает все блоки, занимаемые файлом
};
