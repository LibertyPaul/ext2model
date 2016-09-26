#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <clocale>
#include <Windows.h>
using namespace std;


class Menu{
	vector<string> items;
	string cursor;
	unsigned int marginLeft, marginTop, padding;//������ �� ������ ���� ����� ������� ������� + marginLeft
	HANDLE consoleOut;
	bool numeration;
	bool isCleared;
	int currentCursorPos;
public:
	Menu(){
		setlocale(LC_ALL, "");
		numeration=true;
		isCleared=false;
		currentCursorPos=0;
		cursor="-> ";
		items.push_back("������� ����");
		items.push_back("������� ����");
		items.push_back("�������� ������� � �����");
		items.push_back("��������� ���� ����������");
		items.push_back("�������� ���� ����������");
		items.push_back("������� ����");
		items.push_back("������� ����");
		items.push_back("�������� �����");
		items.push_back("������� �������");
		items.push_back("������� �������");
		items.push_back("�������� �������");
		items.push_back("���������� � ��������");
		items.push_back("������ �����");
		items.push_back("������� �����");
		items.push_back("�������� ����");
		items.push_back("�����");
		

		consoleOut=GetStdHandle(STD_OUTPUT_HANDLE);
		if(consoleOut == INVALID_HANDLE_VALUE)
			throw exception("Menu::Menu() HANDLE �� �������");

		marginLeft=1;
		marginTop=1;
		padding=1;
	}

	~Menu(){
		CloseHandle(consoleOut);
	}

private:
	void flushCursor(){
		COORD cursorCoord={marginLeft, currentCursorPos*(padding+1)+marginTop};
		SetConsoleCursorPosition(consoleOut, cursorCoord);
		for(unsigned int i=0;i<cursor.length();++i)
			cout<<' ';
	}
public:

	void clearScreen(){
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		DWORD count;
		DWORD cellCount;
		COORD homeCoords={0, 0};
		if(!GetConsoleScreenBufferInfo(consoleOut, &csbi)) return;
		cellCount = csbi.dwSize.X *csbi.dwSize.Y;
		if(!FillConsoleOutputCharacter(consoleOut, L' ', cellCount, homeCoords, &count)) return;
		if(!FillConsoleOutputAttribute(consoleOut, csbi.wAttributes, cellCount, homeCoords, &count)) return;
		SetConsoleCursorPosition(consoleOut, homeCoords);
		isCleared=true;
	}
private:
	void writeCursor(){
		COORD cursorCoord={marginLeft, currentCursorPos*(padding+1)+marginTop};
		SetConsoleCursorPosition(consoleOut, cursorCoord);
		cout<<cursor;
	}
public:
	void writeMenu(){
		int itemNumber=0;
		if(isCleared)
			for(auto string : items){
				COORD current={marginLeft + cursor.length(), itemNumber*(padding+1)+marginTop};
				SetConsoleCursorPosition(consoleOut, current);
				if(numeration)cout<<itemNumber+1<<".\t";
				cout<<string;
				++itemNumber;
			}
		writeCursor();
	}

	void setCursorPosition(int pos){
		flushCursor();
		currentCursorPos=pos%items.size();
		writeCursor();
	}

	void cursorInc(){
		flushCursor();
		currentCursorPos=(currentCursorPos+1)%items.size();
		writeCursor();
	}

	void cursorDec(){
		flushCursor();
		currentCursorPos=currentCursorPos-1;
		if(currentCursorPos < 0)
			currentCursorPos=items.size()-1;
		writeCursor();
	}

	int getKursorPos(){
		return currentCursorPos%items.size();
	}

	int getMenuSize(){
		return items.size();
	}

	void gotoxy(int x, int y){
		COORD c={x, y};
		SetConsoleCursorPosition(consoleOut, c);
	}

	string getItem(unsigned int number){
		if(number > items.size())return "";
		return items.at(number);
	}
};