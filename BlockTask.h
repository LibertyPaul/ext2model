#pragma once

using namespace std;
typedef unsigned int uint;
typedef unsigned long int luint;

class BlockTask{//������� �� ������ ����������� �����
	uint blockNumber;
	uint start;//� ������ ����� �������� ������/������
	uint length;//������� ���� ������/������(start + length) < blockSize
	bool overflow;//���� ���� ����� �� ���������� � �������� ����
public:
	static uint blockSize;
	BlockTask(uint blockNumber, uint length, uint start, bool overflow = false);
	uint getBlockNumber();
	uint getStartPoint();
	uint getLength();
	void setOverflow(bool overflow);
	bool isOverflowing();
};
