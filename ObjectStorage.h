#pragma once

using namespace std;
typedef unsigned int uint;

template <typename ObjType>
class ObjectStorage{//обеспечивает работу с областью информации о файлах(FileInfo)
	LocationInfo storage;
	vector<bool> bitmap;

public:
	static HDD *hdd;
	static Cache *cache;
	static FS *fs;

	ObjectStorage();//создание нового хранилища
	ObjectStorage(const LocationInfo &storage, const vector<bool> &bitmap);
	uint getFreePlace();//ищет пустое место посреди файла. если не находит, то добавляет блок
	uint count();//количество хранимых объектов
	vector<uint> getObjectIds();
	uint addObject(ObjType object);
	void updateObject(uint id, ObjType &object);
	ObjType readObject(uint id);
	vector<ObjType> readObjects(const vector<uint> &ids);
	void deleteObject(uint id);
	LocationInfo makeBackup();//сохраняет свое состояние в файл. Перед созданием нового бэкапа нужно удалить старый.

	
};


template class ObjectStorage<FileInfo>;//собстно хранилище
HDD *ObjectStorage<FileInfo>::hdd;
Cache *ObjectStorage<FileInfo>::cache;
FS *ObjectStorage<FileInfo>::fs;

