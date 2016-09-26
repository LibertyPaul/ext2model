
#include "Command.h"
#include <vector>

using namespace std;

#include "Shell.h"


Shell::Shell(){
	commands.emplace_back("mkfile", "s", "Создание файла. mkfile <имя_файла>");
	commands.emplace_back("open", "s", "Открытие файла. open <имя_файла>");
	commands.emplace_back("changepos", "ici", "Изменение позиции в файле. changepos <HANDLE> <B|C|E> <дистанция>");
	commands.emplace_back("read", "iii", "Чтение файла. read <HANDLE> [BEGIN=0] <length>");
	commands.emplace_back("write", "isii", "Запись в файл. write <HANDLE> <text> [start=0] length");
	commands.emplace_back("close", "i", "Закрытие файла. close <HANDLE>");
	commands.emplace_back("delete", "s", "Удаление файла. delete <имя_файла>");
	commands.emplace_back("dir", "", "Отображение файлов в директории. dir [PATH=current]");
	commands.emplace_back("mkdir", "s", "Создание директории. mkdir <имя_директории>");
	commands.emplace_back("rmdir", "s", "Удаление пустой директории. Для удаления не пустой использовать флаг r. rmdir <имя_директории> [r]");
	commands.emplace_back("changedir", "ss", "");//not ready
	commands.emplace_back("info", "s", "Информация о директории.");
	commands.emplace_back("import", "ss", "Импорт файла. import <source> <destination>");
	commands.emplace_back("export", "ss", "Экспорт файла. export <source> <destination>");
	commands.emplace_back("cd", "s", "Изменение текущей директории. cd <имя_директории>");
	commands.emplace_back("exit", "", "Выход.");
}