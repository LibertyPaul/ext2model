#pragma once

typedef unsigned int uint;
typedef unsigned long int luint;

class OpenedFile{
	FileRecord fileRecord;
	int handle;
	luint position;

public:
	static int handleCounter;

	OpenedFile(FileRecord fileRecord, luint position = 0);

	int getHandle();
	FileRecord getFileRecord();
	luint getPosition();
	void setPosition(luint position);
};