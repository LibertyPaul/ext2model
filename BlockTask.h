#pragma once

using namespace std;
typedef unsigned int uint;
typedef unsigned long int luint;

class BlockTask{//задание на чтение конкретного блока
	uint blockNumber;
	uint start;//с какого байта начинать чтение/запись
	uint length;//сколько байт читать/писать(start + length) < blockSize
	bool overflow;//если этот кусок не помещается в исходный файл
public:
	static uint blockSize;
	BlockTask(uint blockNumber, uint length, uint start, bool overflow = false);
	uint getBlockNumber();
	uint getStartPoint();
	uint getLength();
	void setOverflow(bool overflow);
	bool isOverflowing();
};
