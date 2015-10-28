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
#define RESPONSESIZE 15

//end of define flags

#define DELIMITER "GENERAL"
#include "ftd2xx.h"
#include "boost/date_time/local_time/local_time.hpp"
#include "boost/crc.hpp"
#include "D:\ubs\Dev\lib\win32DLib\win32DLib\ucu_fw\src\dllapi\factory.h"
#include "crc32.h"

class StrategyDeployment 
{
private:
	std::string commodFileName;
	std::string folderSaveName;
	std::string zipLocation;
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
	std::list <FT_DEVICE_LIST_INFO_NODE> deviceList;

public:
	enum Commands
	{
		VersionRequest = 1,
		LoadFW = 2,
		LoadCM = 3,  // Загрузка в КМ
		SaveCM = 4,  // Выгрузка КМ в ПК
		DiagnosticEnable = 5, // Включить диагностику
		DiagnosticDisable = 6, // Выключить диагностику
		Configuration = 7, // Запрос конфигурации
		GetChannelSettings = 8, // Запрос настроек каналов
		GetFails = 9, // Запрос отказов
		GetDynamicInfo = 10, // Запрос динамической информации
		Reboot = 11, // Перезагрузка
		StartCalibrate = 12, // Откалибровать входы
		GetCalibrates = 13, // Получить текущие калибровки
		SaveSettings = 14, // Сохранить пользовательские настройки
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
	bool saveFile(const std::string fileName, const std::vector<unsigned char> vecToSave);
	bool saveFile(const std::string fileName, const std::list<std::string> listToSave);
	bool openfile(const std::string fileName);
	bool isFileExists(const std::string& name) const{
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}
	long getFileSize(std::string filename) const{
		struct stat stat_buf;
		int rc = stat(filename.c_str(), &stat_buf);
		return rc == 0 ? stat_buf.st_size : -1;
	}
	bool convert();
	bool validateCurrentConfiguration();
	int getCRC32Commod();
	void addIntToVect(int var, std::vector<unsigned char> &vector);
	void addShortToVect(short var, std::vector <unsigned char> &vector);
	
	//ft_device
	bool loadConfiguration();
	void rebootDevice();
	unsigned int getDevicesCount();
	FT_HANDLE getFirstDeviceHandle();
	FT_HANDLE getDeviceByDescription(const std::string description);
	FT_STATUS sendPacket(FT_HANDLE ftHandle, std::vector<unsigned char> &buffer, DWORD bytesToSend, LPDWORD lpdwBytesWritten);
	FT_STATUS sendCommand(FT_HANDLE ftHandle, Commands command);
	void createPacket(std::vector <unsigned char> &buffer);
	int readResponse(FT_HANDLE ft_handle, unsigned long bytesToRead);
	FT_STATUS closeFTDI(FT_HANDLE ftHandle);
	FT_STATUS loadCM(FT_HANDLE ft_handle);
	FT_STATUS reboot(FT_HANDLE ft_handle); 
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
