#pragma once

using namespace std;
typedef unsigned int uint;

template <typename ObjType>
class ObjectStorage{//������������ ������ � �������� ���������� � ������(FileInfo)
	LocationInfo storage;
	vector<bool> bitmap;

public:
	static HDD *hdd;
	static Cache *cache;
	static FS *fs;

	ObjectStorage();//�������� ������ ���������
	ObjectStorage(const LocationInfo &storage, const vector<bool> &bitmap);
	uint getFreePlace();//���� ������ ����� ������� �����. ���� �� �������, �� ��������� ����
	uint count();//���������� �������� ��������
	vector<uint> getObjectIds();
	uint addObject(ObjType object);
	void updateObject(uint id, ObjType &object);
	ObjType readObject(uint id);
	vector<ObjType> readObjects(const vector<uint> &ids);
	void deleteObject(uint id);
	LocationInfo makeBackup();//��������� ���� ��������� � ����. ����� ��������� ������ ������ ����� ������� ������.

	
};


template class ObjectStorage<FileInfo>;//������� ���������
HDD *ObjectStorage<FileInfo>::hdd;
Cache *ObjectStorage<FileInfo>::cache;
FS *ObjectStorage<FileInfo>::fs;

