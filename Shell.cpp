
#include "Command.h"
#include <vector>

using namespace std;

#include "Shell.h"


Shell::Shell(){
	commands.emplace_back("mkfile", "s", "�������� �����. mkfile <���_�����>");
	commands.emplace_back("open", "s", "�������� �����. open <���_�����>");
	commands.emplace_back("changepos", "ici", "��������� ������� � �����. changepos <HANDLE> <B|C|E> <���������>");
	commands.emplace_back("read", "iii", "������ �����. read <HANDLE> [BEGIN=0] <length>");
	commands.emplace_back("write", "isii", "������ � ����. write <HANDLE> <text> [start=0] length");
	commands.emplace_back("close", "i", "�������� �����. close <HANDLE>");
	commands.emplace_back("delete", "s", "�������� �����. delete <���_�����>");
	commands.emplace_back("dir", "", "����������� ������ � ����������. dir [PATH=current]");
	commands.emplace_back("mkdir", "s", "�������� ����������. mkdir <���_����������>");
	commands.emplace_back("rmdir", "s", "�������� ������ ����������. ��� �������� �� ������ ������������ ���� r. rmdir <���_����������> [r]");
	commands.emplace_back("changedir", "ss", "");//not ready
	commands.emplace_back("info", "s", "���������� � ����������.");
	commands.emplace_back("import", "ss", "������ �����. import <source> <destination>");
	commands.emplace_back("export", "ss", "������� �����. export <source> <destination>");
	commands.emplace_back("cd", "s", "��������� ������� ����������. cd <���_����������>");
	commands.emplace_back("exit", "", "�����.");
}