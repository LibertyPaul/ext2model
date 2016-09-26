#pragma once

typedef unsigned int uint;
typedef long unsigned int luint;
typedef long long int llint;


using namespace std;


class FS{
	static uint const max_path = 255;
	HDD *hdd;
	ObjectStorage<FileInfo> *fileInfoStorage;
	uint currentPosition;
	Cache *cache;
	LocationInfo storageBackup;

	vector<OpenedFile> openedFiles;

	
	vector<wstring> separatePath(wstring fName);
	FileInfo findFile(FileInfo currentDir, wstring fName);

	vector<BlockTask> makeTask(LocationInfo location, luint start, luint length);
	void updateSuperBlock();
	SuperBlock getSuperBlock();

	FileInfo getRootInfo();
	FileRecord getRootRecord();

	FileRecord getDirectory(wstring fName);
	wstring currentDir;
	
	friend class ObjectStorage<FileInfo>;

	//directory stuff
	uint getFileRecordsCount(LocationInfo &dir);
	uint getRealFileRecordsCount(LocationInfo &dir);
	uint getDirFreePlace(LocationInfo &dir);
	uint addFileRecord(LocationInfo &dir, FileRecord &fileRecord);
	void editFileRecord(LocationInfo &dir, FileRecord &fileRecord, uint place);
	void deleteFileRecord(LocationInfo &dir, uint place);
	FileRecord getFileRecord(LocationInfo &dir, uint place);
	FileRecord getFileRecord(wstring fName);
	uint searchFileRecord(LocationInfo &dir, wstring name);
	bool isExist(wstring fName);
	bool isDirEmpty(wstring fName);

	//opened files stuff
	void addOpenedFile(OpenedFile openedFile);
	OpenedFile getOpenedFile(int handle);
	void editOpenedFile(OpenedFile openedFile);
	
	luint readFilePart(LocationInfo &location, luint start, luint length, void *dst);
	luint writeFilePart(LocationInfo &location, luint start, luint length, void *source);

	void showFiles(FileRecord &dirRecord, int offset);
public:
	FS();
	~FS();
	void save();//вызывается для корректного завершения работы

	void createDirectory(wstring dirName);
	void createFile(wstring fName, bool isDir = false);
	int openFile(wstring fName);
	int closeFile(int handle);
	int setPosition(int handle, SetPositionMethod method, llint diff);
	void deleteFile(wstring fName);//удаляет файлы и пустые директории
	luint writeFile(int handle, luint length, void *source);
	luint readFile(int handle, luint length, void *dst);
	void showFiles();

	//searchResult search(wstring query);
	luint importFile(wstring src, wstring dst);
	luint exportFile(wstring src, wstring dst);

	void changeDir(wstring dirName);
	wstring getCurrentDir();
	DirStat getDirStat(wstring fName);

	bool isHandleExist(int handle);

};

