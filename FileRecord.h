#pragma once

typedef unsigned int uint;

const uint maxNameSize = 256;

struct FileRecord{
	wchar_t fName[maxNameSize];//��� �����
	uint id;//����� � ��������� FileInfo
	bool isDir;
};