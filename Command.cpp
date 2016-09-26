

#include <string>
#include "Command.h"

Command::Command(string name, string paramList, string info){
	if(name.empty() || paramList.empty())
		throw exception("empty parameter");

	this->name = name;
	this->paramList = paramList;//TODO: проверка типов
	this->info = info;
}

string Command::getName(){
	return name;
}

string Command::getParamList(){
	return paramList;
}

string Command::getInfo(){
	return info;
}