#pragma once

using namespace std;

typedef unsigned int uint;

const uint magicNumber = 42;

class HDD{
	HANDLE hFile;
	std::wstring fName;

	uint blockSize;
	uint blockCount;
	uint chunkLength;//����. ���������� ������� � �����

	//����(Chunk) == ����, �������� ������� ��������� ������

	uint topLimit;//����� �������� �������� �����
	uint bottomLimit;//����� �������� �������� �����
	uint freeBlockCount;//���������� ��������� ������


	static const uint superBlockPosition = 0;
	static const uint superBlockSize = 1024;
	static const uint chunkStart = 1;
	double overflowPoint;


	vector<Chain> chunkCache;
	TransactionCache transaction;
	vector<FSError> errors;

	vector<void *> readStack;
	vector<uint> blockStack;

public:
	bool isNew;

	HDD(wstring fName);
	~HDD();

protected:
	uint cacheSize();
	int setFileSize();
	int setFilePointer(uint blockNumber);
	int readBlock(uint blockNumber, void *block);
	int writeBlock(uint blockNumber, void *block);
	uint findLastChunk(uint blockNumber=0);//���� ����� ����� � ������ ����� ������ #blockNumber
	uint requestBlock();
	vector<uint> requestBlock(uint count);
	void zeroBlockCache(void *block);
	int fillWithZeros(uint blockNumber);
	int freeBlock(Chain chain);
	int freeBlock(uint blockNumber);
	int uploadChunkCache(uint chunkNumber=0);
	int readFS();//������ ����� ��������� �������� ������� �� �����
	int uploadHalfChunk();
	int downloadHalfChunk();
	int checkOverflow();
	uint chunkCount();
	int optimizeChains(vector<Chain> &chain);
	void setLastError(ErrorCode errorCode);
	void setLastError(ErrorCode errorCode, DWORD ext);
	void setLastError(FSError error);
	int checkIfProtected(vector<uint> blocks);
	int checkIfProtected(uint blockNumber);
	int removeIfAlreadyFree(vector<uint> &blocks);
	

public:
	void *allocateBlock();//�������� ���������� ���� ������ �������� � 1 ����
	int takeBlocks(uint count);
	int freeBlocks(vector<uint> blocks);
	int writeMemory(vector<uint> blocks, void **info);
	void writeMemory(uint blockNumber, void *block);
	int readMemory(uint blockNumber);
	FSError getLastError();
	uint format(uint blockSize, uint blockCount);//returns freeBlockCount
	int applyTransactions();
	int cancelTransactions();
	void dropTransactions();
	vector<void *> getReadStack();
	vector<uint> getBlockStack();
	uint getRootDirectoryBlock();
	void setRootDirectoryBlock(uint blockNumber);
	uint popBlockStack();
	int popReadStack(void *block);
	int frontReadStack(void *block);
	int uploadSuperBlock(void *src);
	int readSuperBlock(void *dst);//������ ����� ���� � ����� ������ ������ ��������� ������

	vector<Chain> getChains();
	uint getBlockSize();
	uint getBlockCount();
	uint getFreeBlockCount();
private:
	void setBlockSize(uint blockSize);
	void setBlockCount(uint blockCount);
	void setFreeBlockCount(uint freeBlockCount);
};
	