#pragma once

typedef unsigned int uint;

const uint maxNameSize = 256;

struct FileRecord{
	wchar_t fName[maxNameSize];//имя файла
	uint id;//номер в хранилище FileInfo
	bool isDir;
};