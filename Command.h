#pragma once
using namespace std;

class Command{
	string name;
	string paramList;//строка со списком параметров(без имени команды). ssi - 3 параметра: 2 строки и число
	string info;
public:
	Command(string name, string paramList, string info);

	string getName();
	string getParamList();
	string getInfo();
};
