#pragma once

#include "FileRecord.h"
#include "OpenedFile.h"

OpenedFile::OpenedFile(FileRecord fileRecord, luint position): fileRecord(fileRecord), position(position){
		handle = handleCounter++;
	}

	int OpenedFile::getHandle(){
		return handle;
	}

	FileRecord OpenedFile::getFileRecord(){
		return fileRecord;
	}

	luint OpenedFile::getPosition(){
		return position;
	}

	void OpenedFile::setPosition(luint position){
		this->position = position;
	}