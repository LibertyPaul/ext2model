#pragma once

#include <Windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include "error_codes.h"

#include "Chain.h"
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "TransactionCache.h"


#include "HDD.h"

uint Transaction_base::blockSize;

	HDD::HDD(wstring fName): fName(fName){
		isNew = false;
		overflowPoint = 0.8;		
		blockSize = 1024;//для чтения супер блока
		blockCount = 1;
		

		hFile = CreateFile(fName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(hFile == INVALID_HANDLE_VALUE)
			if(GetLastError() == ERROR_FILE_NOT_FOUND){
				hFile = CreateFile(fName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
				if(hFile == INVALID_HANDLE_VALUE)
					setLastError(FILE_CREATE_ERROR, GetLastError());
				else
					isNew = true;		
			}
			else
				setLastError(FILE_OPEN_ERROR, GetLastError());
		else 
			if(readFS() == -1)
				isNew = true;
	}

	HDD::~HDD(){
		CloseHandle(hFile);
	}


	uint HDD::getBlockSize(){
		return blockSize;
	}

	uint HDD::getBlockCount(){
		return blockCount;
	}


	void HDD::zeroBlockCache(void *block){
		ZeroMemory(block, blockSize);
	}

	void *HDD::allocateBlock(){
		void *ptr;
		try{
			ptr=operator new(blockSize);
		}
		catch(bad_alloc &ba){
			setLastError(BAD_MEMORY_ALLOCATION);
			return nullptr;
		}
		zeroBlockCache(ptr);
		return ptr;
	}

	uint HDD::cacheSize(){
		uint size=0;
		for(auto ch : chunkCache)
			size += ch.getLength();
		return size;
	}

	int HDD::setFileSize(){
		if(setFilePointer(blockCount) == -1)
			return -1;
		if(SetEndOfFile(hFile) == FALSE){
			setLastError(SET_EOF_ERROR, GetLastError());
			return -1;
		}
		return 0;
	}

	int HDD::setFilePointer(uint blockNumber){
		LARGE_INTEGER pos;
		pos.QuadPart = (long long)blockNumber * blockSize;
		BOOL res=SetFilePointerEx(hFile, pos, nullptr, FILE_BEGIN);
		if(res == FALSE){
			DWORD extCode = GetLastError();
			setLastError(SET_FILE_POINTER_ERROR, extCode);
			throw exception("setFilePointer external error #" + extCode);
		}
		return 0;
	}

	int HDD::readBlock(uint blockNumber, void *block){
		if(blockNumber > blockCount){
			setLastError(BLOCK_DOES_NOT_EXIST);
			throw exception("blockNumber > blockCount");
		}

		setFilePointer(blockNumber);

		DWORD read;
		BOOL readRes=ReadFile(hFile, block, blockSize, &read, nullptr);
		if(readRes == FALSE){
			DWORD extCode = GetLastError();
			setLastError(READFILE_ERROR, extCode);
			throw exception("ReadBlock external error #" + extCode);
		}

		return 0;
	}


	int HDD::writeBlock(uint blockNumber, void *block){
		setFilePointer(blockNumber);
		
		DWORD written;

		BOOL writeRes=WriteFile(hFile, block, blockSize, &written, nullptr);
		if(writeRes == FALSE){
			setLastError(WRITEFILE_ERROR, GetLastError());
			throw exception("writeBlock error #" + WRITEFILE_ERROR);
		}
		
		
		return 0;
	}


	uint HDD::findLastChunk(uint blockNumber){//ищет номер чанка в списке перед чанком #blockNumber
		void *block=allocateBlock();
		if(block == nullptr)
			return 0;

		uint *uint_block = (uint *)block;
		uint current = chunkStart;
		uint last;

		do{
			if(readBlock(current, block) == -1){
				operator delete(block);
				return -1;
			}
			last=current;
			current = uint_block[blockSize / sizeof(uint) - 1];
		}while(current != blockNumber);

		operator delete(block);
		return last;
	}

	uint HDD::requestBlock(){
		if(chunkCache.empty()){
			setLastError(CHUNK_CACHE_IS_EMPTY);
			return 0;
		}
		uint block=chunkCache.back().getBackNumber();
		chunkCache.back().decLength();
		if(chunkCache.back().getLength() == 0)
			chunkCache.pop_back();
		--freeBlockCount;
		return block;
	}

	vector<uint> HDD::requestBlock(uint count){
		vector<uint> result;

		for(uint i=0;i < count;++i){
			result.push_back(chunkCache.back().getBackNumber());
			if(chunkCache.back().decLength() == 0)
				chunkCache.pop_back();
			if(chunkCache.empty())
				if(downloadHalfChunk() == -1){
					for(auto res : result){
						Chain ch(res, 1);
						chunkCache.push_back(ch);
					}
					result.clear();
					return result;
				}
		}
		freeBlockCount-=count;
		return result;
	}


	int HDD::fillWithZeros(uint blockNumber){
		void *block=allocateBlock();
		if(block == nullptr)return -1;
		
		if(writeBlock(blockNumber, block) == -1){
			operator delete(block);
			return -1;
		}

		operator delete(block);
		return 0;
	}


	int HDD::freeBlock(Chain chain){
		chunkCache.push_back(chain);
		freeBlockCount += chain.getLength();
		return 0;
	}

	int HDD::freeBlock(uint blockNumber){
		Chain chain(blockNumber, 1);
		chunkCache.push_back(chain);
		++freeBlockCount;
		return 0;
	}

	int HDD::uploadChunkCache(uint chunkNumber){
		void *block=allocateBlock();
		if(block == nullptr)
			return -1;

		uint *chain = (uint *)block;
		uint i=0;
		for(auto ch : chunkCache){
			chain[i++]=ch.getFrontNumber();
			chain[i++]=ch.getLength();
		}
		if(chunkNumber == 0)
			chunkNumber=findLastChunk();
		if(chunkNumber == 0)
			return -1;

		if(writeBlock(chunkNumber, block) == -1){
			operator delete(block);
			return -1;
		}

		operator delete(block);
		return 0;
	}

	int HDD::optimizeChains(vector<Chain> &chain){
		if(chain.empty()){
			setLastError(CHAIN_IS_EMPTY);
			return 0;
		}
		sort(chain.begin(), chain.end());
		vector<Chain> tempCache(chain);
		chain.clear();
		Chain ch=tempCache.front();
		for(auto i=tempCache.begin() + 1; i != tempCache.end(); ++i)
			if(ch.getBackNumber() + 1 == i->getFrontNumber())
				ch.appendChain(*i);
			else{
				chain.push_back(ch);
				ch=*i;
			}
		chain.push_back(ch);
		return 0;
	}


	int HDD::uploadSuperBlock(void *src = nullptr){
		if(src == nullptr){
			void *src = allocateBlock();
			readBlock(superBlockPosition, src);//Low-level, without transactions
			uint *superBlock = (uint *)src;
			
			superBlock[0] = magicNumber;
			superBlock[1] = blockSize;
			superBlock[2] = blockCount;
			superBlock[3] = freeBlockCount;
		
			if(writeBlock(superBlockPosition, src) == -1){
				operator delete(src);
				return -1;
			}
			
			operator delete(src);
			return 0;
		}
		else
			return writeBlock(superBlockPosition, src);
	}


	int HDD::applyTransactions(){
		vector<uint> freeBlocksBuffer;
		Transaction_base *current = transaction.pop_front();
		while(current != nullptr){
			try{
			switch(current->getType()){

			case FREE_BLOCK:	freeBlocksBuffer.push_back(current->getBlockNumber());
								break;


			case WRITE_INFO:	{
								
								void *block = operator new(blockSize);//auto_ptr использовать должен ты
								current->copyBlock(block);

								void *backup = operator new(blockSize);
								readBlock(current->getBlockNumber(), backup);
								current->saveBlock(backup);
								operator delete(backup);
								
								writeBlock(current->getBlockNumber(), block);
								operator delete(block);
								
								
								}
								break;


			case READ_INFO:		{
									void *block=allocateBlock();
									readBlock(current->getBlockNumber(), block);
									readStack.push_back(block);
								}
								break;


			case GET_BLOCK:		blockStack.insert(blockStack.end(), requestBlock());
								break;

			default:			throw exception("unknown switch value");
			}
			}catch(...){
				cancelTransactions();
				throw;
			}

			current->setApplyed();
			current = transaction.pop_front();
		}

		transaction.clear();

		std::sort(freeBlocksBuffer.begin(), freeBlocksBuffer.end());
		for(auto block : freeBlocksBuffer)
			freeBlock(block);

		if(checkOverflow() == -1)
			return -1;

		if(uploadChunkCache() == -1)
			return -1;

		if(uploadSuperBlock() == -1)
			return -1;

		return 0;
	}

	int HDD::cancelTransactions(){
		Transaction_base *current = transaction.pop_back();
		while(current != nullptr){
			if(current->isApplyed()){
					switch (current->getType()){

					case FREE_BLOCK: break;//nothing to do. Until transaction is over blocks wasn't really freed.

					case GET_BLOCK: for(auto j = blockStack.begin();  j != blockStack.end(); ++j)
										if(current->getBlockNumber() == *j){
											freeBlock(*j);
											blockStack.erase(j);
										}
											
									break;

					case READ_INFO: break;//Reading don't change HDD.

					case WRITE_INFO: {
										void *backup = operator new(blockSize);
										current->copyBlock(backup);

										writeBlock(current->getBlockNumber(), backup);

										operator delete(backup);
									 }
									 break;

					default: throw exception("unknown switch value");
					}
			}
			current = transaction.pop_back();
		}
		transaction.clear();
		for(auto ptr : readStack)
			delete[] ptr;
		readStack.clear();
		blockStack.clear();

		if(checkOverflow() == -1)
			return -1;

		if(uploadChunkCache() == -1)
			return -1;

		if(uploadSuperBlock() == -1)
			return -1;

		return 0;
	}

	void HDD::dropTransactions(){
		transaction.clear();
	}


	uint HDD::format(uint blockSize, uint blockCount){
		this->blockSize = blockSize;
		this->blockCount = blockCount;
		this->freeBlockCount = 0;
		this->chunkLength=blockSize / sizeof(Chain);
		this->topLimit=chunkLength * overflowPoint;
		this->bottomLimit=chunkLength * (1 - overflowPoint);
		Transaction_base::blockSize = blockSize;

		errors.clear();
		CloseHandle(hFile);
		hFile=CreateFile(fName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(setFileSize() == -1)
			return 0;

		chunkCache.clear();
		transaction.clear();
		

		Chain emptyFS(2, blockCount-2);
		freeBlock(emptyFS);

		if(uploadChunkCache(chunkStart) == -1)
			return 0;

		if(uploadSuperBlock() == -1)
			return 0;
		return this->freeBlockCount;
	}

	
	int HDD::readFS(){//читает ранее созданную файловую систему из файла
		uint *superBlock;
		try{
			superBlock = static_cast<uint *>(operator new(blockSize));
		}catch(bad_alloc &ba){
			cerr<<ba.what()<<endl;
			throw;
		}
		readSuperBlock(superBlock);

		if(superBlock[0] != magicNumber){
			setLastError(FILESYSTEM_NOT_FOUND);
			return -1;
		}
		this->blockSize = superBlock[1];
		this->blockCount = superBlock[2];
		this->freeBlockCount = superBlock[3];
		this->chunkLength=blockSize / sizeof(Chain);
		this->topLimit=chunkLength * overflowPoint;
		this->bottomLimit=chunkLength * (1 - overflowPoint);
		Transaction_base::blockSize = blockSize;

		operator delete(superBlock);

		
		void *block=allocateBlock();
		if(block == nullptr)
			return -1;
		
		uint lastChunk=findLastChunk();
		if(lastChunk == 0){
			operator delete(block);
			return -1;
		}

		if(readBlock(lastChunk, block) == -1){
			operator delete(block);
			return -1;
		}

		uint *chunk = (uint *)block;
		for(uint i = 0; chunk[i]; i += sizeof(Chain) / sizeof(uint)){
			Chain ch(chunk[i], chunk[i+1]);
			chunkCache.push_back(ch);
		}

		operator delete(block);	
		return 0;
	}

	int HDD::uploadHalfChunk(){
		while(chunkCache.size() > topLimit){
			void *block=allocateBlock();
			if(block == nullptr)
				return -1;

			uint *halfChunk=(uint *)block;

			
			for(uint i=0 ; i < chunkLength && chunkCache.empty() == false ; i+=sizeof(Chain) / sizeof(uint)){
				halfChunk[i]=chunkCache.back().getFrontNumber();
				halfChunk[i+1]=chunkCache.back().getLength();
				chunkCache.pop_back();
			}
			
			uint newChunk=requestBlock();
			if(newChunk == 0){
				operator delete(block);
				return -1;
			}
			halfChunk[blockSize / sizeof(uint) - 1] = newChunk;
	
			if(writeBlock(findLastChunk(), block) == -1){
				operator delete(block);
				return -1;
			}
			operator delete(block);
		}
	
		if(uploadChunkCache() == -1)
			return -1;

		return 0;
	}

	int HDD::downloadHalfChunk(){
		while(chunkCache.size() < bottomLimit && chunkCount() > 1){
			uint last=findLastChunk();
			if(last == 0)
				return -1;
			uint penultimate=findLastChunk(last);
			if(penultimate == 0)
				return -1;

			void *block=allocateBlock();
			if(block == nullptr)
				return -1;
			uint *halfChunk=(uint *)block;

			if(readBlock(penultimate, block) == -1){//считываем предпоследний чанк
				operator delete(block);
				return -1;
			}
			
			for(uint i=0; halfChunk[i]; i += sizeof(Chain) / sizeof(uint)){
				Chain ch(halfChunk[i], halfChunk[i+1]);
				chunkCache.push_back(ch);
			}
			operator delete(block);
		
			
			Chain lastChunk(last, 1);//освободим последний чанк
			chunkCache.push_back(lastChunk);
			if(optimizeChains(chunkCache) == -1)
				return -1;
			if(uploadChunkCache(penultimate) == -1)
				return -1;
		}
		return 0;
	}


	int HDD::checkOverflow(){
		if(chunkCache.size() > topLimit)
			if(uploadHalfChunk() == -1)
				return -1;
		if(chunkCache.size() < bottomLimit && chunkCount() > 1)
			if(downloadHalfChunk() == -1)
				return -1;
		return 0;
	}

	uint HDD::chunkCount(){
		uint count=1;
		uint current=findLastChunk();
		if(current == 0)
			return 0;
		while(current != chunkStart){
			current=findLastChunk(current);
			if(current == 0)
				return 0;
			++count;
		}

		return count;
	}

	void HDD::setLastError(ErrorCode errorCode){
			FSError error(errorCode);
			errors.push_back(error);
	}

	void HDD::setLastError(ErrorCode errorCode, DWORD externalErrorCode){
		FSError error(errorCode, externalErrorCode);
		errors.push_back(error);
	}

	void HDD::setLastError(FSError error){
		errors.push_back(error);
	}
		

	int HDD::takeBlocks(uint count){
		if(count > freeBlockCount){
			setLastError(NOT_ENOUGH_BLOCKS);
			throw exception("not enough blocks");
		}
		
		for(uint i = 0; i < count; ++i){
			Transaction_base tr(GET_BLOCK, 0);
			transaction.push_back(&tr);
		}

		return 0;
	}

	int HDD::checkIfProtected(vector<uint> blocks){
		vector<uint> protectedBlocks;
		uint current = findLastChunk();
		if(current == 0)
			return -1;
		while(current != chunkStart){
			protectedBlocks.push_back(current);
			current=findLastChunk(current);
			if(current == 0)
				return -1;
		}
		protectedBlocks.push_back(chunkStart);
		
		
		for(auto block : blocks){
			auto i=find(protectedBlocks.begin(), protectedBlocks.end(), block);
			if(i != protectedBlocks.end())
				return 1;
		}
		return 0;
	}

	int HDD::checkIfProtected(uint blockNumber){
		vector<uint> protectedBlocks;
		uint current = findLastChunk();
		if(current == 0)
			return -1;

		while(current != chunkStart){
			protectedBlocks.push_back(current);
			current=findLastChunk(current);
			if(current == 0)
				return -1;
		}
		protectedBlocks.push_back(chunkStart);
		if(find(protectedBlocks.begin(), protectedBlocks.end(), blockNumber) != protectedBlocks.end())
			return 1;
		return 0;
	}

	int HDD::removeIfAlreadyFree(vector<uint> &blocks){
		vector<Chain> cache;
		vector<uint> temp;

		
		void *block=allocateBlock();
		if(block == nullptr)
			return -1;
		uint currentChunk=0;
		do{
			currentChunk=findLastChunk(currentChunk);
			if(currentChunk == 0){
				operator delete(block);
				return -1;
			}

			if(readBlock(currentChunk, block) == -1){
				operator delete(block);
				return -1;
			}
			uint *chunk=(uint *)block;
			for(auto n : blocks){
				bool exist=false;
				for(uint i=0; chunk[i]; i += sizeof(Chain) / sizeof(uint)){
					Chain ch(chunk[i], chunk[i+1]);
					if(ch.isInRange(n)){
						exist=true;
						break;
					}
				}
				if(exist == false)
					temp.push_back(n);
			}

			if(currentChunk == 0)
				return -1;

			zeroBlockCache(block);
			blocks=temp;
			temp.clear();
		}while(currentChunk != chunkStart);

		delete[] block;
		return 0;
	}





	int HDD::freeBlocks(vector<uint> blocks){
		if(blocks.empty())return 0;
		if(checkIfProtected(blocks)){
			setLastError(BLOCK_IS_PROTECTED);
			throw exception("Reading protected block attempt");
		}
		
		removeIfAlreadyFree(blocks);
		std::sort(blocks.begin(), blocks.end());

		for(auto block : blocks){
			Transaction_base tr(FREE_BLOCK, block);
			transaction.push_back(&tr);
		}		

		return 0;
	}

	int HDD::writeMemory(vector<uint> blocks, void **info){
		if(checkIfProtected(blocks)){
			setLastError(BLOCK_IS_PROTECTED);
			return -1;
		}
		uint i=0;
		for(auto block : blocks)
			writeMemory(block, info[i++]);
		return 0;
	}

	void HDD::writeMemory(uint blockNumber, void *block){
		Transaction_info tr(WRITE_INFO, blockNumber, block);
		transaction.push_back(&tr);
	}


	int HDD::readMemory(uint blockNumber){
		if(blockNumber > blockCount){
			setLastError(BLOCK_DOES_NOT_EXIST);
			throw exception("BLOCK_DOES_NOT_EXIST");
		}

		if(checkIfProtected(blockNumber)){
			setLastError(BLOCK_IS_PROTECTED);
			throw exception("BLOCK_IS_PROTECTED");
		}

		Transaction_base tr(READ_INFO, blockNumber);
		transaction.push_back(&tr);

		return 0;
	}

	FSError HDD::getLastError(){
		FSError error;
		if(errors.empty())
			return error;

		error = errors.back();
		errors.pop_back();
		return error;
	}

	uint HDD::popBlockStack(){
		if(blockStack.empty()){
			setLastError(STACK_IS_EMPTY);
			return 0;
		}

		uint result = blockStack.back();
		blockStack.pop_back();
		return result;
	}

	int HDD::popReadStack(void *block){
		if(readStack.empty()){
			setLastError(STACK_IS_EMPTY);
			throw exception("reading empty stack");
		}

		memcpy_s(block, blockSize, readStack.back(), blockSize);
		readStack.pop_back();

		return 0;
	}

	int HDD::frontReadStack(void *block){
		if(readStack.empty()){
			setLastError(STACK_IS_EMPTY);
			return 0;
		}

		memcpy_s(block, blockSize, readStack.front(), blockSize);
		readStack.erase(readStack.begin());
		return 0;
	}

	vector<uint> HDD::getBlockStack(){
		return blockStack;
	}

	vector<void *> HDD::getReadStack(){
		return readStack;
	}

	uint HDD::getFreeBlockCount(){
		return freeBlockCount;
	}

	int HDD::readSuperBlock(void *buf){
		if(readBlock(superBlockPosition, buf) == -1){
			return -1;
		}
		return 0;
	}


	void HDD::setBlockSize(uint blockSize){
		this->blockSize = blockSize;
	}

	void HDD::setBlockCount(uint blockCount){
		this->blockCount = blockCount;
	}

	void HDD::setFreeBlockCount(uint freeBlockCount){
		this->freeBlockCount = freeBlockCount;
	}