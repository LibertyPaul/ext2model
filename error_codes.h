#pragma once

enum ErrorCode{
	OK,
	NOT_ENOUGH_BLOCKS,
	BAD_MEMORY_ALLOCATION,
	SET_FILE_POINTER_ERROR,
	BLOCK_DOES_NOT_EXIST,
	CREATE_FILE_ERROR,
	BAD_FILE,
	READFILE_ERROR,
	WRITEFILE_ERROR,
	CANT_DOWNLOAD_ANOTHER_CHUNK,
	BLOCK_IS_PROTECTED,
	BLOCK_IS_FREE,
	SET_EOF_ERROR,
	CHAIN_IS_EMPTY,
	CHUNK_CACHE_IS_EMPTY,
	EMPTY_REQUEST,
	STACK_IS_EMPTY,
	FILESYSTEM_NOT_FOUND,
	READ_SUPERBLOCK_ERROR,
	FILE_CREATE_ERROR,
	FILE_OPEN_ERROR,
	NO_FREE_BLOCKS
};



class FSError{
	ErrorCode internalCode;
	DWORD externalCode;
public:
	FSError(ErrorCode internalCode=OK, DWORD externalCode=0): internalCode(internalCode), externalCode(externalCode){
	}

	ErrorCode getInternalCode(){
		return internalCode;
	}

	DWORD getExternalCode(){
		return externalCode;
	}
};