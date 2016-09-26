#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <algorithm>
#include <string>
#include <exception>
#include <memory>

#include "DirStat.h"
#include "SetPositionMethod.h"
#include "Chain.h"
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "TransactionCache.h"
#include "error_codes.h"



#include "HDD.h"
class FS;
#include "BlockCache.h"
#include "Cache.h"

#include "FifteenFields.h"
#include "error_codes.h"
#include "LocationInfo.h"
#include "SuperBlock.h"
#include "FileInfo.h"
#include "BlockTask.h"
#include "FileRecord.h"
#include "OpenedFile.h"

#include "ObjectStorage.h"


#include "FS.h"

Cache *FifteenFields::cache;
HDD *FifteenFields::hdd;
uint FifteenFields::TIBcapacity;
uint FifteenFields::DIBcapacity;
uint FifteenFields::IBcapacity;
uint FifteenFields::directCapacity;
uint FifteenFields::blockSize;

HDD *Cache::hdd;

HDD *BlockCache::hdd;

HDD *LocationInfo::hdd;

int OpenedFile::handleCounter;


	FS::FS(){
		currentDir = L"/";
		wstring fName=L"file.dat";
		//cout<<"Input filename: ";
		//wcin>>fName;
		
		
		OpenedFile::handleCounter = 0;
		ObjectStorage<FileInfo>::fs = this;

		hdd=new HDD(fName);
		Cache::hdd = hdd;
		BlockCache::hdd = hdd;
		FifteenFields::hdd = hdd;
		ObjectStorage<FileInfo>::hdd = hdd;
		LocationInfo::hdd = hdd;


		cache = new Cache();
		FifteenFields::cache = cache;
		ObjectStorage<FileInfo>::cache = cache;	

		
		
		ErrorCode error = hdd->getLastError().getInternalCode();
		if(error == OK){//HDD создан без ошибок
			if(hdd->isNew){
				uint blockSize = 4096;
				uint blockCount = 20000;

				wcout<<"******FS not found in "<<fName<<". ******"<<endl<<endl;
				cout<<"Input new FS parameters :"<<endl;
				cout<<"-block size: ";
				cin>>blockSize;
				cout<<"-block count: ";
				cin>>blockCount;
				
				if(hdd->format(blockSize, blockCount) == 0)
					cout<<"HDD formatting error #"<<hdd->getLastError().getInternalCode()<<endl;
				
				BlockTask::blockSize = blockSize;
				FifteenFields::blockSize = blockSize;
				FifteenFields::IBcapacity = blockSize / sizeof(uint);
				FifteenFields::DIBcapacity = FifteenFields::IBcapacity * FifteenFields::IBcapacity;
				FifteenFields::TIBcapacity = FifteenFields::DIBcapacity * FifteenFields::IBcapacity;
				FifteenFields::directCapacity = 12;

				FileInfo root;
				fileInfoStorage = new ObjectStorage<FileInfo>();
				fileInfoStorage->addObject(root);
				updateSuperBlock();

			}
			else{//считываем хранилище
				SuperBlock superBlock = getSuperBlock();

				BlockTask::blockSize = superBlock.blockSize;
				FifteenFields::blockSize = superBlock.blockSize;
				FifteenFields::IBcapacity = superBlock.blockSize / sizeof(uint);
				FifteenFields::DIBcapacity = FifteenFields::IBcapacity * FifteenFields::IBcapacity;
				FifteenFields::TIBcapacity = FifteenFields::DIBcapacity * FifteenFields::IBcapacity;
				FifteenFields::directCapacity = 12;

				

				if(superBlock.storageBackup.getBlockCount() > 0){//если бэкап не пуст
					uint bitmapLength;//в битах
					readFilePart(superBlock.storageBackup, 0, sizeof(uint), &bitmapLength);//4 байта - размер bitmap, n байт - bitmap in chars, LocationInfo
					
					uint bitmapSize = bitmapLength / 8 + (bitmapLength % 8 ? 1 : 0);//в байтах
					char *bitmap_buffer = new char[bitmapSize];
					readFilePart(superBlock.storageBackup, sizeof(uint), bitmapSize, bitmap_buffer);
					vector<bool> bitmap;
					for(uint i = 0; i < bitmapLength; ++i)
						bitmap.push_back((bitmap_buffer[i / 8] >> (i % 8)) & 0x01);
					delete[] bitmap_buffer;

					LocationInfo storage_file;
					readFilePart(superBlock.storageBackup, sizeof(uint) + bitmapSize, sizeof(LocationInfo), &storage_file);
					
					fileInfoStorage = new ObjectStorage<FileInfo>(storage_file, bitmap);
				}
				else{
					FileInfo root;
					fileInfoStorage = new ObjectStorage<FileInfo>();
					fileInfoStorage->addObject(root);
					updateSuperBlock();
				}
			}				
				
		}
		else 
			throw exception("HDD error #"+error);
	}


	FS::~FS(){
		save();
		delete fileInfoStorage;
		delete cache;
		delete hdd;
	}

	void FS::save(){
		updateSuperBlock();
	}
	
	void FS::updateSuperBlock(){
		SuperBlock superBlock;
		superBlock.magicNumber = magicNumber;
		superBlock.blockSize = hdd->getBlockSize();
		superBlock.blockCount = hdd->getBlockCount();
		superBlock.freeBlockCount = hdd->getFreeBlockCount();
		this->storageBackup.freeBlocks();
		this->storageBackup = fileInfoStorage->makeBackup();
		superBlock.storageBackup = this->storageBackup;

		void *superBuffer;
		try{
			superBuffer = operator new(hdd->getBlockSize());
		}catch(bad_alloc &ba){
			cerr<<ba.what()<<endl;
			throw;
		}

		memcpy_s(superBuffer, hdd->getBlockSize(), &superBlock, sizeof(SuperBlock));
		hdd->uploadSuperBlock(superBuffer);
		operator delete(superBuffer);
		
	}

	SuperBlock FS::getSuperBlock(){
		void *super_buffer;
		try{
			super_buffer = operator new(hdd->getBlockSize());
		}catch(bad_alloc &ba){
			cerr<<ba.what()<<endl;
			throw;
		}

		hdd->readSuperBlock(super_buffer);
		SuperBlock superBlock;
		memcpy_s(&superBlock, sizeof(SuperBlock), super_buffer, sizeof(SuperBlock));
		operator delete(super_buffer);
		return superBlock;
	}


	vector<wstring> FS::separatePath(wstring fName){//tested
		size_t found = fName.find_first_of(L'/');
		size_t prev = found + 1;
		if(found != 0)
			cerr<<"error";
		else
			found = fName.find_first_of(L'/', found + 1);
		
		vector<wstring> path;
		while(found != string::npos){
			path.push_back(fName.substr(prev, found - prev));
			if(path.back().size() > max_path)
				throw exception("Filename too large");
			prev = found + 1;
			found = fName.find_first_of(L'/', found + 1);
		}
		path.push_back(fName.substr(prev, fName.length() - prev));
		if(path.back().size() > max_path)
				throw exception("Filename too large");
		
		return path;
	}

	vector<BlockTask> FS::makeTask(LocationInfo location, luint start, luint length){//формирует список заданий на поблочное чтение, и, если необходимо, добавляет блоки в файл
		vector<BlockTask> task;
		uint blockSize = hdd->getBlockSize();
		

		while(length > 0){
			uint currentLength;
			if(length > blockSize - start % blockSize)
				currentLength = blockSize - start % blockSize;
			else
				currentLength = length;
				BlockTask blockTask(location.resolve15fields(start / blockSize), currentLength, start % blockSize);
				task.push_back(blockTask);
			start += task.back().getLength();
			length -= task.back().getLength();
		}

		return task;
	}

	luint FS::readFilePart(LocationInfo &location, luint start, luint length, void *dst){
		if(start + length > location.getRealFileSize())
			return readFilePart(location, start, location.getRealFileSize() - start, dst);//рекурсия может быть опасна
		
		vector<BlockTask> readTask = makeTask(location, start, length);

		char *ptr = (char *)dst;

		for(auto rT : readTask)
			if(hdd->readMemory(rT.getBlockNumber()) == -1){//запрашиваем набор блоков на чтение
				hdd->dropTransactions();//need check
				throw exception("HDD read memory error #" + hdd->getLastError().getInternalCode());
			}

		if(hdd->applyTransactions() == -1)//читаем
			throw exception("HDD apply transactions error #" + hdd->getLastError().getInternalCode());
		
		for(auto rT : readTask){
			char *block = new char[hdd->getBlockSize()];
			hdd->frontReadStack(block);
			if(block == nullptr)
				throw exception("block isn't exist. HDD error code #" + hdd->getLastError().getInternalCode());
			memcpy_s(ptr, rT.getLength(), block + rT.getStartPoint(), rT.getLength());
			ptr += rT.getLength();
			delete[] block;
		}
		return length;
	}


	luint FS::writeFilePart(LocationInfo &location, luint start, luint length, void *source){
		if(start + length > location.getRealFileSize())
			if(location.resize(start+length) == -1)//закончились свободные блоки
				length = location.getBlockCount() * hdd->getBlockSize() - start;
			

		vector<BlockTask> writeTask = makeTask(location, start, length);

		char *src = static_cast<char *>(source);//для перемещения по массиву
		uint blockSize = hdd->getBlockSize();
		char *buffer = new char[blockSize];
		for(auto task : writeTask){
			if(task.getLength() != blockSize){//если записываем часть блока, нужно считать весь блок, изменить часть и записать целиком
				hdd->readMemory(task.getBlockNumber());
				hdd->applyTransactions();

				buffer = static_cast<char *>(operator new(hdd->getBlockSize()));
				hdd->popReadStack(buffer);
			}

			memcpy_s(buffer + task.getStartPoint(), task.getLength(), src, task.getLength());
			hdd->writeMemory(task.getBlockNumber(), buffer);
			src += task.getLength();
		}
		delete[] buffer;

		if(hdd->applyTransactions() == -1)
			throw exception("HDD apply transactions error #" + hdd->getLastError().getInternalCode());
		return length;
	}

	FileInfo FS::getRootInfo(){
		return fileInfoStorage->readObject(0);
	}

	FileRecord FS::getRootRecord(){
		FileRecord root;
		root.id = 0;
		root.fName[0] = L'\0';
		root.isDir = true;
		return root;
	}

	uint FS::getFileRecordsCount(LocationInfo &dir){//включая пустые места посередине
		return dir.getRealFileSize() / sizeof(FileRecord);
	}

	uint FS::getRealFileRecordsCount(LocationInfo &dir){//только действительные записи
		uint pseudoCount = getFileRecordsCount(dir);
		uint realCount = 0;
		for(uint i = 0; i < pseudoCount; ++i){
			FileRecord fileRecord = getFileRecord(dir, i);
			if(fileRecord.id)
				++realCount;
		}
		return realCount;
	}


	uint FS::getDirFreePlace(LocationInfo &dir){
		uint count = getFileRecordsCount(dir);
		for(uint i = 0; i < count; ++i){
			FileRecord fileRecord;
			readFilePart(dir, i * sizeof(FileRecord), sizeof(FileRecord), &fileRecord);//можно оптимизировать, читая сразу несколько структур
			if(fileRecord.id == 0)//id == 0 признак пустого места. Структура с id = 0 (root) не существует
				return i;
		}

		return count;//если пустых мест посреди файла нет - дописываем в конец
	}


	uint FS::addFileRecord(LocationInfo &dir, FileRecord &fileRecord){
		uint place = getDirFreePlace(dir);
		writeFilePart(dir, place * sizeof(FileRecord), sizeof(FileRecord), &fileRecord);
		return place;
	}
		

	void FS::editFileRecord(LocationInfo &dir, FileRecord &fileRecord, uint place){
		writeFilePart(dir, place * sizeof(FileRecord), sizeof(FileRecord), &fileRecord);
	}

	void FS::deleteFileRecord(LocationInfo &dir, uint place){
		FileRecord fileRecord;
		fileRecord.id = 0;
		editFileRecord(dir, fileRecord, place);
	}

	FileRecord FS::getFileRecord(LocationInfo &dir, uint place){
		FileRecord fileRecord;
		readFilePart(dir, place * sizeof(FileRecord), sizeof(FileRecord), &fileRecord);
		return fileRecord;
	}

	uint FS::searchFileRecord(LocationInfo &dir, wstring name){//
		uint count = getFileRecordsCount(dir);

		for(uint i = 0; i < count; ++i){
			FileRecord current = getFileRecord(dir, i);
			if(name.compare(current.fName) == 0)
				return i;
		}
		return 0xFFFFFFFF;
	}

	FileRecord FS::getFileRecord(wstring fName){
		if(fName == L"/")
			return getRootRecord();
		if(fName.back() == L'/')
			fName.pop_back();
		FileRecord dirAboveRecord = getDirectory(fName);
		FileInfo dirAboveInfo = fileInfoStorage->readObject(dirAboveRecord.id);
		vector<wstring> path = separatePath(fName);
		uint id = searchFileRecord(dirAboveInfo.location, path.back());
		if(id == 0xFFFFFFFF)
			throw exception("file does not exist");

		FileRecord result = getFileRecord(dirAboveInfo.location, id);

		return result;
	}

	
	FileRecord FS::getDirectory(wstring fName){//path-empty unsafe
		if(fName.back() == L'/')
			fName.pop_back();//костыль

		vector<wstring> path = separatePath(fName);

		FileInfo current = getRootInfo();
		FileRecord record = getRootRecord();
		for(auto i = path.begin(); i != path.end() - 1; ++i){
			int dirRecordPlace = searchFileRecord(current.location, *i);

			record = getFileRecord(current.location, dirRecordPlace);
			if(record.isDir == false)
				throw exception("wrong path");

			current = fileInfoStorage->readObject(record.id);
		}

		return record;
	}

	bool FS::isExist(wstring fName){
		vector<wstring> path = separatePath(fName);
		return searchFileRecord(fileInfoStorage->readObject(getDirectory(fName).id).location, path.back()) != 0xFFFFFFFF;
	}

	bool FS::isDirEmpty(wstring fName){
		FileRecord dirRecord = getFileRecord(fName);
		FileInfo dirInfo = fileInfoStorage->readObject(dirRecord.id);
		uint realRecordsCount = getRealFileRecordsCount(dirInfo.location);
		return realRecordsCount == 0;
	}


	void FS::createFile(wstring fName, bool isDir){
		if(fName[0] != L'/')
			fName = currentDir + fName;

		if(isExist(fName))
			throw exception("File with this name is already exists.");
		FileRecord dirAboveRecord;
		dirAboveRecord = getDirectory(fName);
		

		FileInfo dirAboveInfo = fileInfoStorage->readObject(dirAboveRecord.id);

		FileRecord newFile;
		vector<wstring> path = separatePath(fName);
		wcscpy(newFile.fName, path.back().c_str());
		newFile.isDir = isDir;
		FileInfo fileInfo;
		newFile.id = fileInfoStorage->addObject(fileInfo);

		addFileRecord(dirAboveInfo.location, newFile);

		fileInfoStorage->updateObject(dirAboveRecord.id, dirAboveInfo);
	}

	void FS::createDirectory(wstring fName){
		if(fName[0] != L'/')
			fName = currentDir + fName;

		createFile(fName, true);
	}


	void FS::showFiles(FileRecord &dirRecord, int offset = 0){
		FileInfo dir = fileInfoStorage->readObject(dirRecord.id);
		uint count = getFileRecordsCount(dir.location);

		for(uint i = 0; i < count; ++i){//проход по папкам
			try{
				FileRecord fileRecord = getFileRecord(dir.location, i);
				if(fileRecord.id != 0)
					if(fileRecord.isDir){
						for(uint offsetCounter = 0; offsetCounter < offset; ++offsetCounter)wcout<<L"\t";
						wcout<<fileRecord.fName<<"/"<<endl;
						showFiles(fileRecord, offset + 1);
					}
			}catch(exception &exc){
				cerr<<"### ERROR "<<exc.what()<<" ###"<<endl;
			}
		}

		for(uint i = 0; i < count; ++i){//порход по файлам
			try{
				FileRecord fileRecord = getFileRecord(dir.location, i);
				if(fileRecord.id != 0)
					if(fileRecord.isDir == false){
						for(uint offsetCounter = 0; offsetCounter < offset; ++offsetCounter)wcout<<L"\t";
						wcout<<fileRecord.fName<<endl;
					}
			}catch(exception &exc){
				cerr<<"### ERROR "<<exc.what()<<" ###"<<endl;
			}
		}
	}

	void FS::addOpenedFile(OpenedFile openedFile){
		openedFiles.push_back(openedFile);
	}

	OpenedFile FS::getOpenedFile(int handle){
		for(auto file : openedFiles)
			if(file.getHandle() == handle)
				return file;
		throw exception("Handle not found.");
	}

	void FS::editOpenedFile(OpenedFile openedFile){
		for(auto &file : openedFiles)
			if(file.getHandle() == openedFile.getHandle()){
				file = openedFile;
				return;
			}
		throw exception("Handle not found.");
	}

	int FS::openFile(wstring fName){
		if(fName[0] != L'/')
			fName = currentDir + fName;

		if(isExist(fName) == false)
			return -1;
		FileRecord fileRecord;
		try{

			fileRecord = getFileRecord(fName);
			if(fileRecord.isDir)
				return -1;
			OpenedFile openedFile(fileRecord);
			addOpenedFile(openedFile);
			return openedFile.getHandle();
		
		}catch(exception &exc){
			cerr<<exc.what()<<endl;
			return -1;
		}
	}
		
	int FS::closeFile(int handle){
		for(auto i = openedFiles.begin(); i != openedFiles.end(); ++i)
			if(i->getHandle() == handle){
				openedFiles.erase(i);
				return 0;
			}
		throw exception("Handle not found");
	}

	int FS::setPosition(int handle, SetPositionMethod method, llint diff){
		try{
			OpenedFile openedFile = getOpenedFile(handle);
			FileInfo openedFileInfo = fileInfoStorage->readObject(openedFile.getFileRecord().id);
			luint fileSize = openedFileInfo.location.getRealFileSize();
			

			switch(method){
			case BEGIN:		openedFile.setPosition(diff);
							break;

			case END:		if(fileSize - diff < 0)
								throw out_of_range("position less than 0");
							openedFile.setPosition(fileSize - diff);
							break;

			case CURRENT:	if(openedFile.getPosition() + diff < 0)
								throw out_of_range("position less than 0");
							openedFile.setPosition(openedFile.getPosition() + diff);
							break;

			default:		throw invalid_argument("Invalid SetPositionMethod");
			}

			editOpenedFile(openedFile);
			return 0;
		}catch(exception &exc){
			cerr<<exc.what()<<endl;
			return -1;
		}
	}

	void FS::deleteFile(wstring fName){
		if(fName[0] != L'/')
			fName = currentDir + fName;
		FileRecord targetFile = getFileRecord(fName);
		if(targetFile.isDir)
			if(isDirEmpty(fName) == false)
				throw exception("Directory isn't empty.");
			else
				if(currentDir.compare(targetFile.fName) == 0)
					currentDir = L"/";
					
		FileInfo targetFileInfo = fileInfoStorage->readObject(targetFile.id);
		targetFileInfo.location.freeBlocks();
		fileInfoStorage->deleteObject(targetFile.id);
		FileRecord dirAboveRecord = getDirectory(fName);
		FileInfo dirAboveInfo = fileInfoStorage->readObject(dirAboveRecord.id);

		vector<wstring> path = separatePath(fName);
		uint place = searchFileRecord(dirAboveInfo.location, path.back());
		if(place == 0xFFFFFFFF)
			throw exception("File is not exist");
			
		deleteFileRecord(dirAboveInfo.location, place);
	}
	
	luint FS::readFile(int handle, luint length, void *dst){
		OpenedFile openedFile = getOpenedFile(handle);
		FileInfo openedFileInfo = fileInfoStorage->readObject(openedFile.getFileRecord().id);
		luint read = readFilePart(openedFileInfo.location, openedFile.getPosition(), length, dst);
		openedFile.setPosition(openedFile.getPosition() + length);
		editOpenedFile(openedFile);
		return read;
	}

	
	luint FS::writeFile(int handle, luint length, void *source){
		OpenedFile openedFile = getOpenedFile(handle);
		FileInfo openedFileInfo = fileInfoStorage->readObject(openedFile.getFileRecord().id);
		luint written = writeFilePart(openedFileInfo.location, openedFile.getPosition(), length, source);
		fileInfoStorage->updateObject(openedFile.getFileRecord().id, openedFileInfo);
		openedFile.setPosition(openedFile.getPosition() + length);
		editOpenedFile(openedFile);
		return written;
	}

	
	luint FS::importFile(wstring src, wstring dst){
		if(dst[0] != L'/')
			dst = currentDir + dst;
		createFile(dst);
		
		int hDst = openFile(dst);
		HANDLE hSrc = CreateFile(src.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(hSrc == INVALID_HANDLE_VALUE)
			if(GetLastError() == ERROR_FILE_NOT_FOUND)
				throw exception("Source file not found");
			else
				throw exception("Source file opening error");

		DWORD read;
		luint written = 0;
		const uint bufSize = 1024;
		void *buf = operator new(bufSize);
		do{
			ReadFile(hSrc, buf, bufSize, &read, nullptr);
			written += writeFile(hDst, read, buf);
		}while(read);

		operator delete(buf);

		closeFile(hDst);
		CloseHandle(hSrc);

		return written;
	}

	luint FS::exportFile(wstring src, wstring dst){
		if(src[0] != L'/')
			src = currentDir + src;

		int hSrc = openFile(src);

		HANDLE hDst = CreateFile(dst.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if(hDst == INVALID_HANDLE_VALUE)
			throw exception("Destination file opening error");

		luint read;
		DWORD written;
		luint writtenSum = 0;

		const uint bufSize = 1024;
		unique_ptr<char> buf(new char[bufSize]);

		do{
			read = readFile(hSrc, bufSize, buf.get());
			WriteFile(hDst, buf.get(), read, &written, nullptr);
			writtenSum += written;
		}while(read == bufSize);

		CloseHandle(hDst);
		closeFile(hSrc);

		return writtenSum;
	}

	void FS::showFiles(){
		FileRecord folder = getFileRecord(currentDir);
		wcout<<folder.fName<<L"/"<<endl;
		showFiles(folder, 1);
	}


	void FS::changeDir(wstring dirName){
		if(dirName[0] != L'/')
			dirName = currentDir + dirName;

		FileRecord dir = getFileRecord(dirName);
		
		if(dir.isDir){
			if(dirName.back() != L'/')
				dirName.push_back(L'/');
			currentDir = dirName;
		}
		else
			throw exception("not a directory");
	}

	wstring FS::getCurrentDir(){
		return currentDir;
	}

	DirStat FS::getDirStat(wstring fName){
		if(fName[0] != L'/')
			fName = currentDir + fName;

		FileRecord fileRecord = getFileRecord(fName);
		if(fileRecord.isDir == false)
			throw exception("not a directory");
		else
			if(fName.back() != L'/')
				fName.push_back(L'/');
		FileInfo fileInfo = fileInfoStorage->readObject(fileRecord.id);
		DirStat stat;
		stat.size = fileInfo.location.getBlockCount() * hdd->getBlockSize();
		stat.realSize = fileInfo.location.getRealFileSize();
		stat.innerSize = 0;
		stat.fileCount = 0;
		stat.dirCount = 0;

		uint recordCount = getFileRecordsCount(fileInfo.location);

		for(uint i = 0; i < recordCount; ++i){
			FileRecord current;
			try{
				current = getFileRecord(fileInfo.location, i);
			}catch(exception &ex){
				throw runtime_error("DirStat FS::getDirStat(wstring fName) error");
			}
			if(current.isDir){
				DirStat belowStat = getDirStat(fName + current.fName);
				stat.innerSize += belowStat.innerSize + belowStat.size;
				++stat.dirCount;
			}
			else{
				FileInfo belowFileInfo = fileInfoStorage->readObject(current.id);
				stat.innerSize += belowFileInfo.location.getBlockCount() * hdd->getBlockSize();
				++stat.fileCount;
			}
		}

		return stat;
	}


	bool FS::isHandleExist(int handle){
		for(auto openedFile : openedFiles)
			if(openedFile.getHandle() == handle)
				return true;

		return false;
	}