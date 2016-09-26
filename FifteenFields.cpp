#pragma once
#include <exception>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>

#include "Chain.h"
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "TransactionCache.h"
#include "error_codes.h"


#include "HDD.h"
#include "BlockCache.h"
#include "Cache.h"





#include "FifteenFields.h"



FifteenFields::FifteenFields(){
		for(uint &p : directPointers)
			p=0;
		IB=DIB=TIB=0;
	}

uint FifteenFields::resolveIB(uint IBblock, uint fileBlockNumber){
		fileBlockNumber -= directCapacity;//пропускаем 12 полей прямого доступа
		if(IBblock == 0)
			throw std::exception("IB = 0");
		void *IB = cache->readBlock(IBblock);
		uint *chunk=(uint *)IB;
		uint blockNumber = chunk[fileBlockNumber];
		operator delete(IB);
		return blockNumber;
	}

uint FifteenFields::resolveDIB(uint DIBblock, uint fileBlockNumber){
		fileBlockNumber -= directCapacity + IBcapacity;
		if(DIBblock == 0)
			throw std::exception("DIB = 0");
		void *DIB = cache->readBlock(DIBblock);
		uint *chunk=(uint *)DIB;
		uint IBblock = chunk[fileBlockNumber / IBcapacity];
		operator delete(DIB);
		return resolveIB(IBblock, fileBlockNumber % IBcapacity);
	}

uint FifteenFields::resolveTIB(uint fileBlockNumber){
		fileBlockNumber -= directCapacity + IBcapacity + DIBcapacity;
		if(TIB == 0)
			throw std::exception("TIB = 0");
		void *TIBblock = cache->readBlock(TIB);
		uint *chunk=(uint *)TIBblock;
		uint DIBblock = chunk[fileBlockNumber / DIBcapacity];
		operator delete(TIBblock);
		return resolveDIB(DIBblock, fileBlockNumber % DIBcapacity);
	}


uint FifteenFields::resolveFifteenFields(uint fileBlockNumber){
		if(fileBlockNumber < directCapacity)
			return directPointers[fileBlockNumber];

		if(fileBlockNumber <= directCapacity + IBcapacity){
			try{
				return resolveIB(IB, fileBlockNumber);
			}catch(std::exception &exc){
				std::cerr<<exc.what()<<std::endl;
				exit(1);
			}
		}

		if(fileBlockNumber <= directCapacity + IBcapacity + DIBcapacity){
			try{
				return resolveDIB(DIB, fileBlockNumber);
			}catch(std::exception &exc){
				std::cerr<<exc.what()<<std::endl;
				exit(1);
			}
		}

		if(fileBlockNumber <= directCapacity + IBcapacity + DIBcapacity + TIBcapacity){
			try{
				return resolveTIB(fileBlockNumber);
			}catch(std::exception &exc){
				std::cerr<<exc.what()<<std::endl;
				exit(1);
			}
		}

		throw std::exception("15fields overflow");
		return 0;
	}


int FifteenFields::editFieldIB(uint IB, uint position, uint field){
		void *block = cache->readBlock(IB);
		uint *chunk = (uint *)block;

		chunk[position] = field;

		cache->writeBlock(IB, block);

		operator delete(block);

		return 0;
	}


int FifteenFields::editFieldDIB(uint DIB, uint position, uint field){
		void *block = cache->readBlock(DIB);
		uint *chunk = (uint *)block;

		if(chunk[position / IBcapacity] == 0){
			hdd->takeBlocks(1);
			hdd->applyTransactions();
			chunk[position / IBcapacity] = hdd->popBlockStack();
		}

		editFieldIB(chunk[position / IBcapacity], position % IBcapacity, field);

		operator delete(block);
		return 0;
	}


int FifteenFields::editFieldTIB(uint position, uint field){
		void *block = cache->readBlock(TIB);
		uint *chunk = (uint *)block;

		if(chunk[position / DIBcapacity] == 0){
			hdd->takeBlocks(1);
			hdd->applyTransactions();
			chunk[position / DIBcapacity] = hdd->popBlockStack();
		}

		editFieldDIB(chunk[position / DIBcapacity], position % DIBcapacity, field);

		operator delete(block);
		return 0;
	}


int FifteenFields::editField(uint blockCount, uint field){
		uint lastBlock = blockCount;
		if(lastBlock < directCapacity)
			this->directPointers[lastBlock] = field;
		else{
			if(this->IB == 0){
				hdd->takeBlocks(1);
				hdd->applyTransactions();
				this->IB = hdd->popBlockStack();
			}
			lastBlock -= directCapacity;
			if(lastBlock < IBcapacity)
				editFieldIB(IB, lastBlock, field);
			else{
				if(this->DIB == 0){
					hdd->takeBlocks(1);
					hdd->applyTransactions();
					this->DIB = hdd->popBlockStack();
				}
				lastBlock -= IBcapacity;
				if(lastBlock < DIBcapacity)
					editFieldDIB(DIB, lastBlock, field);
				else{
					if(this->TIB == 0){
						hdd->takeBlocks(1);
						hdd->applyTransactions();
						this->TIB = hdd->popBlockStack();
					}
					lastBlock -= DIBcapacity;
					if(lastBlock < TIBcapacity)
						editFieldTIB(lastBlock, field);
					else
						return -1;//файл больше допустимого размера
				}
			}
		}
		return 0;
	}

uint FifteenFields::IBfileSize(uint IBblock){
		void *IB = cache->readBlock(IBblock);
		uint *chunk=(uint *)IB;

		uint i;
		for(i=0; i < IBcapacity; ++i)
			if(chunk[i] == 0)
				break;
		operator delete(IB);
		return i;
	}


uint FifteenFields::DIBfileSize(uint DIBblock){
		void *DIB = cache->readBlock(DIBblock);
		uint *chunk=(uint *)DIB;

		uint i;
		for(i=0; i < IBcapacity && chunk[i]; ++i);//количество занятых IB
		uint size = (i-1) * IBcapacity;//i-1 - количество заполненных блоков IB
		size += IBfileSize(i-1);
		return size;
	}


uint FifteenFields::TIBfileSize(){
		uint *chunk = (uint *)cache->readBlock(TIB);
		uint i;
		for(i=0; i < IBcapacity && chunk[i]; ++i);
		uint size = (i-1)*DIBcapacity;
		size += DIBfileSize(i-1);
		return size;
	}


uint FifteenFields::fileSize(){//how many blocks file occupy
		uint size = 0;
		for(uint i=0; i < directCapacity; ++i)
			if(directPointers[i] != 0)
				++size;
		if(IB != 0)
			size += IBfileSize(IB);
		if(DIB != 0)
			size += DIBfileSize(DIB);
		if(TIB != 0)
			size += TIBfileSize();

		return size;
	}