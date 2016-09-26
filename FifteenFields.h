#pragma once

class FS;
typedef unsigned int uint;

class FifteenFields{
public:
	static uint blockSize;
	static uint directCapacity;
	static uint IBcapacity;
	static uint DIBcapacity;
	static uint TIBcapacity;
	static Cache *cache;
	static HDD *hdd;
private:
	uint directPointers[12];
	uint IB;
	uint DIB;
	uint TIB;

public:

	FifteenFields();
	uint resolveFifteenFields(uint fileBlockNumber);
	int editField(uint blockCount, uint field);
	uint fileSize();

private:
	uint resolveIB(uint IBblock, uint fileBlockNumber);
	uint resolveDIB(uint DIBblock, uint fileBlockNumber);
	uint resolveTIB(uint fileBlockNumber);
	int editFieldIB(uint IB, uint position, uint field);
	int editFieldDIB(uint DIB, uint position, uint field);
	int editFieldTIB(uint position, uint field);
	uint IBfileSize(uint IBblock);
	uint DIBfileSize(uint DIBblock);
	uint TIBfileSize();
};
