#include "menu.h"

#include <string>
#include <iostream>
#include <vector>
#include <Windows.h>
#include <algorithm>
#include <string>
#include <exception>
#include <memory>

#include <conio.h>

#include "Chain.h"
#include <exception>
#include "TransactionType.h"
#include "Transaction_base.h"
#include "Transaction_info.h"
#include "error_codes.h"
#include "TransactionCache.h"


#include "DirStat.h"
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

#include "ObjectStorage.h"
#include "OpenedFile.h"

#include "SetPositionMethod.h"
#include "FS.h"

#include <istream>
using namespace std;


void createFile(FS *fs){
	wcout<<L"Имя файла: ";
	wstring fName;
	wcin>>fName;
	if(fName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	fs->createFile(fName);
}

void openFile(FS *fs){
	wcout<<L"Имя файла: ";
	wstring fName;
	wcin>>fName;

	if(fName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	int handle = fs->openFile(fName);
	
	if(handle == -1)
		wcout<<L"Ошибка. Файл не открыт"<<endl;
	else
		wcout<<L"Файл открыт. HANDLE: "<<handle<<endl;

}

void changePosition(FS *fs){
	wcout<<L"HANDLE: ";
	int handle;
	wcin>>handle;
	if(fs->isHandleExist(handle) == false){
		wcout<<L"HANDLE не найден"<<endl;
		return;
	}

	wcout<<L"Как изменять позицию? from begin, from end, from current [b/e/c]: ";
	wchar_t type;
	wcin>>type;

	SetPositionMethod method;
	switch(type){
	case L'B':
	case L'b':method = BEGIN; break;

	case L'E':
	case L'e':method = CURRENT; break;

	case L'C':
	case L'c':method = END; break;
	default: wcout<<L"Некорректный параметр"<<endl; _getch(); return;
	}

	llint diff;
	wcout<<L"Расстояние (байт): ";
	wcin>>diff;

	fs->setPosition(handle, method, diff);
	

}

void read(FS *fs){
	wcout<<L"HANDLE: ";
	int handle;
	wcin>>handle;
	if(fs->isHandleExist(handle) == false){
		wcout<<L"HANDLE не найден"<<endl;
		return;
	}


	wcout<<L"length: ";
	luint length;
	wcin>>length;

	char *buf = new char[length + 1];
	luint read;

	read = fs->readFile(handle, length, buf);
	
	buf[read] = '\0';

	wcout<<L"Считано "<<read<<L" байт."<<endl;
	cout<<buf;

	delete[] buf;
}

	


void write(FS *fs){
	wcout<<L"HANDLE: ";
	int handle;
	wcin>>handle;
	if(fs->isHandleExist(handle) == false){
		wcout<<L"HANDLE не найден"<<endl;
		return;
	}

	wcout<<L"length: ";
	luint length;
	wcin>>length;

	char *buf = new char[length + 1];

	wcout<<L"Input text:"<<endl;
	cin>>buf;

	luint written = fs->writeFile(handle, length, buf);

	wcout<<L"Записано "<<written<<L" байт."<<endl;
	delete[] buf;
}

void close(FS *fs){
	wcout<<L"HANDLE: ";
	int handle;
	wcin>>handle;
	if(fs->isHandleExist(handle) == false){
		wcout<<L"HANDLE не найден"<<endl;
		return;
	}

	fs->closeFile(handle);

	wcout<<L"Файл закрыт"<<endl;
}

void deleteFile(FS *fs){
	wcout<<L"Имя файла: ";
	wstring fName;
	wcin>>fName;
	if(fName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	fs->deleteFile(fName);

	wcout<<L"Файл удален"<<endl;
}


void show(FS *fs){
	fs->showFiles();
}

void createDir(FS *fs){
	wcout<<L"Имя каталога: ";
	wstring dirName;
	wcin>>dirName;
	if(dirName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	fs->createDirectory(dirName);
}

void deleteDir(FS *fs){
	wcout<<L"Имя каталога: ";
	wstring dirName;
	wcin>>dirName;
	if(dirName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	fs->deleteFile(dirName);
}


void import(FS *fs){
	wcout<<L"Имя импортируемого файла: ";
	wstring srcName;
	wcin>>srcName;

	wcout<<L"Имя создаваемого файла: ";
	wstring dstName;
	wcin>>dstName;
	if(dstName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	luint size = fs->importFile(srcName, dstName);

	wcout<<L"Импортировано "<<size<<" байт"<<endl;
}

void exportFile(FS *fs){
	wcout<<L"Имя экспортируемого файла: ";
	wstring srcName;
	wcin>>srcName;
	if(srcName.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	wcout<<L"Имя создаваемого файла: ";
	wstring dstName;
	wcin>>dstName;

	luint size = fs->exportFile(srcName, dstName);
	
	wcout<<L"Экспортировано "<<size<<" байт"<<endl;
}

void changePath(FS *fs){
	wcout<<L"Current path: "<<fs->getCurrentDir()<<endl;
	wcout<<L"New path: ";
	wstring newPath;
	wcin>>newPath;
	if(newPath.find_first_of(L"*?") != string::npos){
		wcout<<L"invalid characters in string(* ?)"<<endl;
		return;
	}

	fs->changeDir(newPath);
	wcout<<L"Path successfully changed"<<endl<<L"Current path: "<<fs->getCurrentDir()<<endl;
}

void dirInfo(FS *fs){
	wstring dirName = fs->getCurrentDir();
	DirStat dirStat = fs->getDirStat(dirName);

	wcout<<L"Имя каталога: "<<fs->getCurrentDir()<<endl;

	wcout<<L"Размер директории: "<<dirStat.realSize<<L" байт"<<endl;
	wcout<<L"Директория занимает: "<<dirStat.size<<L" байт"<<endl;
	wcout<<L"Кодичество файлов: "<<dirStat.fileCount<<endl;
	wcout<<L"Количество каталогов: "<<dirStat.dirCount<<endl;
	wcout<<L"Размер вложений: "<<dirStat.innerSize<<L" байт"<<endl;
}




int main(){
	setlocale(LC_ALL, "");
	Menu *menu=new Menu();
	
	FS *fs;
	try{
		fs = new FS();
		

		menu->clearScreen();
		while(true){
			menu->writeMenu();
			switch((char)_getch()){
				case VK_RETURN: 
					menu->clearScreen();
					try{
						switch(menu->getKursorPos()){
						case 0:createFile(fs);break;
						case 1:openFile(fs);break;
						case 2:changePosition(fs);break;
						case 3:read(fs); break;
						case 4:write(fs); break;
						case 5:close(fs); break;
						case 6:deleteFile(fs); break;
						case 7:show(fs); break;
						case 8:createDir(fs); break;
						case 9:deleteDir(fs); break;
						case 10:break;
						case 11:dirInfo(fs); break;
						case 12:import(fs);break;
						case 13:exportFile(fs); break;
						case 14:changePath(fs); break;
						case 15:goto exit;
						}
					}catch(bad_alloc &ba){
						cerr<<ba.what()<<endl;
					}catch(exception &exc){
						cerr<<exc.what()<<endl;
					}
					_getch();
					
					menu->clearScreen();
					break;
				case -32:
					switch((char)_getch()){
						case 72:
							menu->cursorDec();
							break;
						case 80:
							menu->cursorInc();
							break;
					}
			}
	}




	}catch(exception &exc){
		cerr<<exc.what()<<endl;
	}catch(FSError &error){
		cerr<<"HDD error #"<<error.getInternalCode()<<endl;
	}catch(ErrorCode &ec){
		cerr<<"HDD error #"<<ec<<endl;
	}catch(...){
		cerr<<"Unknown exception"<<endl;
	}
	
	_getch();
	exit: delete fs;

}
