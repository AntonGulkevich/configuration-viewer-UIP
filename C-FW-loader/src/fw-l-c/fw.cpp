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
		cout << "������: ���������� �� ����������, ��� �� ��������.\n\
�������� ��� ���������, ������������ ����������.\n\
��������� ���������� �� ����������." << endl;
		system("pause");
		return -1;
	}
	if (devCount > 1)
	{
		cout << "������: ������ ���� ���������� ������ ���� ����������." << endl;
		system("pause");
		return -1;
	}
	if (argc == 2)
	{
		std::string pathToCommodFile(argv[1]);
		if (!isFileExitWinXp(pathToCommodFile))
		{
		std:cout << "����� � ����� ������ �� ����������";
			system("pause");
			return -1;
		}
		StrategyDeployment* manager = new StrategyDeployment();
		manager->setFirmWareFileName(pathToCommodFile);
		if (!manager->loadFirmWare())
		{
			cout << "\n������: �� ������� ���������������� ��������� �������� ��.\n";
			system("pause");
			return -1;
		}
		cout << "\n�� ������� ���������. �������� �������!";
		manager->saveLog();
		delete manager;
		return 0;

	}
	cout << "������ - �������� ������. ������ ���� ������ ������ ���� �� ������������ ��." << endl;
	system("pause");
	return -1;
}

