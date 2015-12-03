#include <string.h>
#include <tchar.h>
#include "StrategyDeployment .h"
#include <iostream>
#include <fstream>
using namespace std;

bool isFileExitWinXp(const std::string& filename) {
	if (FILE *file = fopen(filename.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

ifstream::pos_type filesize(const char* filename)
{
	ifstream in(filename, std::ios::binary | std::ios::ate);
	return in.tellg();
}


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "RUS");
	int devCount = StrategyDeployment::getUBSKDevicesCount();

	if (!devCount)
	{
		cout << "Ошибка: устройство не подключено, или не отвечает.\n\
Закройте все программы, использующие устройство.\n\
Проверьте подключено ли устройство." << endl;
		system("pause");
		return -1;
	}
	if (devCount > 1)
	{
		cout << "Ошибка: должно быть подключено только одно устройство." << endl;
		system("pause");
		return -1;
	}
	if (argc == 2)
	{
		std::string pathToCommodFile(argv[1]);
		if (!isFileExitWinXp(pathToCommodFile))
		{
		std:cout << "Файла с таким именем не существует";
			system("pause");
			return -1;
		}
		StrategyDeployment* manager = new StrategyDeployment();
		manager->setFirmWareFileName(pathToCommodFile);
		if (!manager->loadFirmWare())
		{
			cout << "\nОшибка: не удалось инициализировать процедуру загрузки ПО.\n";
			system("pause");
			return -1;
		}
		cout << "\nПО успешно загружено. Приятных полетов!";
		manager->saveLog();
		delete manager;
		return 0;

	}
	cout << "Ошибка - неверный формат. Должен быть введен полный путь до загружаемого ПО." << endl;
	system("pause");
	return -1;
}

