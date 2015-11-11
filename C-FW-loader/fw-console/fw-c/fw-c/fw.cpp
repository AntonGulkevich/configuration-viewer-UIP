#include <string.h>
#include <tchar.h>
#include "StrategyDeployment .h"
#include <iomanip>
using namespace std;

bool isFileExists(const std::string& fileName);

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "RUS");
	if (StrategyDeployment::getDevicesCount() != 1)
	{
		cout << "Ошибка: должно быть подключено только одно устройство." <<endl;
		system("pause");
		return -1;
	}
	if (argc == 2) {
		std::string pathToCommodFile(argv[1]);
		if (!isFileExists(pathToCommodFile)) {
			std:cout << "Файла с таким именем не существует";
			system("pause");
			return -1;			
		}
		StrategyDeployment *manager = new StrategyDeployment();
		manager->setFirmWareFileName(pathToCommodFile);
		manager->loadFirmWare();
		manager->saveLog();
		delete manager;
		return 0;
	}
	cout << "Ошибка - неверный формат. Должен быть введен полный путь до загружаемого ПО." << endl;
	system("pause");
	return -1;
	
}

bool isFileExists(const std::string& fileName)
{
	struct stat buffer;
	return (stat(fileName.c_str(), &buffer) == 0);
}
int fileSize(const std::string& fileName)
{
	struct stat buf;
	if (stat(fileName.c_str(), &buf) != 0)
		return -1;
	return buf.st_size;
}