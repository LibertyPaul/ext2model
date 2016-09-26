#pragma once
#include <Windows.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>

#include "Chain.h"
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "TransactionCache.h"
#include "error_codes.h"

#include "HDD.h"


#include "DirStat.h"
#include "FileRecord.h"
#include "BlockCache.h"
#include "Cache.h"
#include "FifteenFields.h"
#include "LocationInfo.h"
#include "FileInfo.h"
#include "BlockTask.h"
#include "SuperBlock.h"
#include "OpenedFile.h"
template <typename ObjType>
class ObjectStorage;
#include "SetPositionMethod.h"
#include "FS.h"

#include "ObjectStorage.h"

	template <typename ObjType>
	ObjectStorage<ObjType>::ObjectStorage(){//создание нового хранилища
	}

	template <typename ObjType>
	ObjectStorage<ObjType>::ObjectStorage(const LocationInfo &storage, const vector<bool> &bitmap): storage(storage), bitmap(bitmap){
	}

	template <typename ObjType>
	uint ObjectStorage<ObjType>::getFreePlace(){
		for(auto i=bitmap.begin(); i!=bitmap.end(); ++i)
			if(*i == false)
				return i - bitmap.begin();
		
		storage.appendField();
		bitmap.resize(storage.getBlockCount() * hdd->getBlockSize() / sizeof(ObjType), false);
		return getFreePlace();
	}

	template <typename ObjType>
	uint ObjectStorage<ObjType>::count(){
		uint count = 0;
		for(auto b : bitmap)
			if(b)++count;
		return count;
	}

	template <typename ObjType>
	vector<uint> ObjectStorage<ObjType>::getObjectIds(){
		vector<uint> result;
		uint n = 0;
		for(auto b : bitmap){
			if(b)result.push_back(n);
			++n;
		}
		return result;
	}


	template <typename ObjType>
	uint ObjectStorage<ObjType>::addObject(ObjType object){
		uint id = getFreePlace();
		bitmap.at(id) = true;
		updateObject(id, object);
		return id;
	}

	template <typename ObjType>
	void ObjectStorage<ObjType>::updateObject(uint id, ObjType &object){
		fs->writeFilePart(storage, id * sizeof(ObjType), sizeof(ObjType), &object);
	}

	template <typename ObjType>
	ObjType ObjectStorage<ObjType>::readObject(uint id){
		ObjType object;
		bool busy;
		
		busy = bitmap.at(id);
		
		if(busy == false)
			throw exception("object not exist");

		fs->readFilePart(storage, id * sizeof(ObjType), sizeof(ObjType), &object);
		return object;
	}

	template <typename ObjType>
	vector<ObjType> ObjectStorage<ObjType>::readObjects(const vector<uint> &ids){
		vector<ObjType> result;
		ObjType object;

		for(auto id : ids)
			result.push_back(readObject(id));

		return result;
	}


	template <typename ObjType>
	void ObjectStorage<ObjType>::deleteObject(uint id){
		if(id == 0)
			throw exception("root deleting attempt");
		try{
			bitmap.at(id) = false;
		}catch(out_of_range &out){
			cerr<<out.what()<<endl;
			return;
		}
	}

	template <typename ObjType>
	LocationInfo ObjectStorage<ObjType>::makeBackup(){
		LocationInfo backup;

		uint bitmapLength = bitmap.size();

		uint bitmapSize = bitmapLength / 8 + (bitmapLength % 8 ? 1 : 0);
		char *bitmap_backup;
		try{
			bitmap_backup = new char[bitmapSize];
		}catch(bad_alloc &ba){
			cerr<<ba.what()<<endl;
			return backup;
		}
		for(uint i = 0; i < bitmapSize; ++i)
			bitmap_backup[i] = 0;

		for(uint i = 0; i < bitmapLength; ++i)
			bitmap_backup[i / 8] |= (bitmap.at(i) ? 1 : 0) << (i % 8);
		
		fs->writeFilePart(backup, 0, sizeof(uint), &bitmapLength);
		fs->writeFilePart(backup, sizeof(uint), bitmapSize, bitmap_backup);
		fs->writeFilePart(backup, sizeof(uint) + bitmapSize, sizeof(LocationInfo), &storage);

		delete[] bitmap_backup;

		return backup;
	}	