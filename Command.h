#pragma once
using namespace std;

class Command{
	string name;
	string paramList;//������ �� ������� ����������(��� ����� �������). ssi - 3 ���������: 2 ������ � �����
	string info;
public:
	Command(string name, string paramList, string info);

	string getName();
	string getParamList();
	string getInfo();
};
