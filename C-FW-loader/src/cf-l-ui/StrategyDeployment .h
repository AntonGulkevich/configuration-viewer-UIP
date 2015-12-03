#ifndef STRATEGYDEPLOYMENT_H
#define STRATEGYDEPLOYMENT_H

#define EMULATION

#include <string>
#include <list>
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <boost/regex.hpp>

#define HEADER_SIZE 256
#define CLOCK_FILE_SIZE 64
#define EEPROMFILE_SIZE 128*1024
#undef  EM_DEBUG

//define flags
#define STARTFLAG 0xABABABAB
#define BODYFLAG 0x05CE23F8
#define ENDFLAG 0x3F344A21
#define FORGOTTENFLAG 0xC077770D
#define COMMANDFLAG 0x03DF3409
#define WRONGSIZE 0xDEAD1024
#define WRONGCRC 0xDEADCCCC
#define OKREPLY 0x3f344A20
#define RESPONSESIZE 64*1024

//end of define flags

#define DELIMITER "GENERAL"
#include "ftd2xx.h"
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/crc.hpp"
#include "ucu_fw\src\dllapi\factory.h"
#include "crc32.h"

#pragma comment(lib, "ftd2xx.lib")
class StrategyDeployment 
{
private:
	std::string commodFileName;
	std::string folderSaveName;
	std::string zipLocation;
	std::string lastConfName;
	bool parseEnabled;
	bool zipEnabled;
	bool saveToSameFolder;
	bool createCompressedFile;
	short zipCompressionLevel;//1-9
	

	FILE* commodFile;
	long commodFileSize;
	std::list<std::string> logList;

	struct Fat {
		unsigned int address;
		unsigned int size;
	};
	//ft_device
	unsigned int currentFTDIDevice;
	//enum Commands
	//{
	//	None = 0,
	//	VersionRequest = 1 << 0,
	//	LoadFW = 1 << 1,
	//	LoadCM = 1 << 2,
	//	SaveCM = 1 << 3,
	//	DiagnosticEnable = 1 << 4,
	//	DiagnosticDisable = 1 << 5
	//}
public:
	enum Commands
	{
		VersionRequest = 0x01,
		LoadFW = 0x02,
		LoadCM = 0x03,  // Загрузка в КМ    /*error*/
		SaveCM = 0x04,  // Выгрузка КМ в ПК
		DiagnosticEnable = 0x05, // Включить диагностику
		DiagnosticDisable = 0x06, // Выключить диагностику
		Configuration = 0x07, // Запрос конфигурации
		GetChannelSettings = 0x08, // Запрос настроек каналов
		GetFails = 0x09, // Запрос отказов
		GetDynamicInfo = 0x0A, // Запрос динамической информации
		Reboot = 0x0B, // Перезагрузка
		StartCalibrate = 0x0C, // Откалибровать входы
		GetCalibrates = 0x0D, // Получить текущие калибровки
		SaveSettings = 0x0E, // Сохранить пользовательские настройки
	};
	explicit StrategyDeployment(const std::string commodFileName_);//constructor
	//get
	std::string getCommodFileName()	{return commodFileName;}//return last opened commod.bin full adress
	short getZipCompressionLevel() const { return zipCompressionLevel; }
	std::string getZipLocation() const  { return zipLocation; }
	bool isParseEnabked() const  { return parseEnabled; }
	bool isZipEnabled() const  { return zipEnabled; }
	bool isSaveToSameFolder() const  { return saveToSameFolder; }
	bool isCreateCompressedFile()const { return createCompressedFile; }
	std::string getLastConfName() { return lastConfName; }
	
	//set
	void setZipLocation(const std::string & location) { zipLocation = location; }
	void setzipCompressionLevel(short level) { zipCompressionLevel = level; }
	void setParse(bool state) { parseEnabled = state;}
	void setZip(bool state) { zipEnabled = state; }
	void zetSaveToSameFolder(bool state) { saveToSameFolder = state; }
	void setCreateCompressedFile(bool state) { createCompressedFile = state; }

	//compression
	bool zip(const std::string &sourceFileName, const std::string zippedFileName);
	bool unzip(const std::string zippedFileName) const;
	bool to_zip();

	//operations with files
	bool saveFile(const std::string fileName, unsigned char *buffer, long size, const std::string &param);
	bool saveFile(const std::string fileName, const std::string &buffer, long size, const std::string &param);
	bool saveFile(const std::string fileName, const std::vector<unsigned char> vecToSave);
	bool saveFile(const std::string fileName, const std::list<std::string> listToSave);
	bool openfile(const std::string fileName);
	inline bool isFileExists(const std::string& filename) {
		if (FILE *file = fopen(filename.c_str(), "r")) {
			fclose(file);
			return true;
		}
		else {
			return false;
		}
	}
	inline long getFileSize(const std::string& filename)
	{
		std::ifstream in(filename.c_str(), std::ios::binary | std::ios::ate);
		return static_cast<long>(in.tellg());
	}
	bool convert();
	bool validateCurrentConfiguration();
	int getCRC32Commod();
	void addIntToVect(int var, std::vector<unsigned char> &vector);
	void addShortToVect(short var, std::vector <unsigned char> &vector);
	
	//ft_device
	void setFTDIdevice(int number);
	void setFTDIDevice(char * serialNumber);
	bool loadConfiguration();
	bool loadConfiguration(FT_HANDLE ft_handle);
	bool loadConfiguration(unsigned int deviceNumber);
	void rebootDevice();
	static unsigned int getDevicesCount();
	static void getSerialNumber(int devideNum, char* serialNumber);
	static void getDeviceDesrc(int deviceNum, char * descr);
	FT_HANDLE getFirstDeviceHandle();
	FT_HANDLE getDeviceByDescription(const std::string description);
	FT_STATUS sendPacket(FT_HANDLE ftHandle, std::vector<unsigned char> &buffer, DWORD bytesToSend, LPDWORD lpdwBytesWritten);
	FT_HANDLE getDeviceHandle(unsigned int DeviceNumber);
	FT_STATUS sendCommand(FT_HANDLE ftHandle, Commands command);
	void createPacket(std::vector <unsigned char> &buffer);
	int readResponse(FT_HANDLE ft_handle, unsigned int size);
	FT_STATUS closeFTDI(FT_HANDLE ftHandle);
	FT_STATUS loadCM(FT_HANDLE ft_handle);
	FT_STATUS reboot(FT_HANDLE ft_handle); 
	FT_STATUS sendSimpleCommand(FT_HANDLE ft_handle, Commands command);
	FT_STATUS versionRequest();
	FT_STATUS loadFW();	
	FT_STATUS saveCM();
	FT_STATUS diagnosticEnable();
	FT_STATUS diagnosticDisable();
	FT_STATUS configuration();
	FT_STATUS getChannelSettings();
	FT_STATUS getFails(); 
	FT_STATUS getDynamicInfo();	
	FT_STATUS startCalibrate();
	FT_STATUS getCalibrates(); 
	FT_STATUS saveSettings();
	bool setFTDISettings(FT_HANDLE ft_handle);


	//log
	void saveLog();
	void showLog();
	//interface
	bool execute();
	~StrategyDeployment();	
};
void replaceAll(std::string& str, const std::string& from, const std::string& to);
typedef Factory * (__stdcall *DLLGETFACTORY)(void);
#endif STRATEGYDEPLOYMENT_H
