#include <string.h>
#include <tchar.h>
#include "StrategyDeployment .h"
using namespace std;

bool isFileExists(const std::string& fileName);

int main(int argc, char* argv[])
{
	if (argc == 1) {
		setlocale(LC_ALL, "RUS");
		std::string pathToCommodFile(argv[1]);
		if (!isFileExists(pathToCommodFile)) {
		std:cout << (L"Файла с таким именем не существует");
			return -1;
			StrategyDeployment *manager = new StrategyDeployment();
			auto isOK = manager->loadFirmWare();
			manager->saveLog();
			delete manager;
			return 0;
		}
		cout << _T("Должен быть введен полный путь до загружаемого ПО.");
		return -1;

	}
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