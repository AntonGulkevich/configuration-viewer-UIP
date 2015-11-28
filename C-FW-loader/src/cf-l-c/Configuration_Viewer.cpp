#include "StrategyDeployment .h"

using std::cout;
using std::endl;
using std::string;
using std::wstring;

bool isLittleEndian()
{
	short int number = 0x1;
	auto numPtr = reinterpret_cast<char*>(&number);
	return (numPtr[0] == 1);
}

bool isFileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

void showHelp()
{
	cout << "\n[commodFileName]\t\t(destination of commod.bin)\n\
[c]\t\t\t\t(convert)\n\
[1-9]\t\t\t\t(zip level if compression needed)\n\
[zipLocation]\t\t\t(destination of 7z.exe)\n\
[0 or 1]\t\t\t(log 0 - show 1 - show&save)\n\n\
W: if the destination way contains spaces you ought to enclose it in quotes!\n\n\
example : \"D:\commod.bin\" c 7 \"D:\Program Files (x86)\7-Zip\7z.exe\" \n";
	system("PAUSE");
}

int main(int argc, char* argv[])
{
	/*commodFileName(destination of commod.bin)
	c(convert)
	7(zip level if compression needed [1-9])
	zipLocation(destination of 7z.exe)
	log 0 - show 1 - show&save
	example: "D:\commod.bin" c 7 "D:\Program Files (x86)\7-Zip\7z.exe"*/
	if (argc > 1)
	{
		setlocale(LC_ALL, "RUS");
		std::string pathToCommodFile(argv[1]);
		if (!isFileExists(pathToCommodFile))
		{
			cout << "\n Error: commod file is not exist!";
			return -1;
		}
		StrategyDeployment *manager = new StrategyDeployment(pathToCommodFile);
		if (argc >= 2) {
			if (std::string(argv[2]) != "c")
			{
				cout << "\n Error: wrong parameters!";
				showHelp();
				return -1;
			}
			manager->setParse(true);
		}
		if (argc >= 3) {
			int zipCompressionLevel = atoi(argv[3]);
			if (zipCompressionLevel <= 0 || zipCompressionLevel > 9)
			{
				cout << "\n Error: wrong parameters!";
				showHelp();
				return -1;
			}
			manager->setZip(true);
			manager->setzipCompressionLevel(zipCompressionLevel);
			manager->setCreateCompressedFile(true);
		}
		if (argc >= 4) {
			std::string zipLocation(argv[4]);
			if (!isFileExists(zipLocation))
			{
				cout << "\n Error: 7z.exe is not exist!";
				return -1;
			}
			manager->setZipLocation(zipLocation);
		}
		manager->execute();
		if (argc >= 5) {
			if (atoi(argv[5]) == 1)
				manager->saveLog();
			manager->showLog();
		}
		delete manager;
		return 0;
	}
	showHelp();
	return 0;
}


