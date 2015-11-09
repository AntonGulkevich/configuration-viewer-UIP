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
#define FW_FLAG 0x12345678
#define FW_HEADER_SIZE 12
#define FW_PACKET_SIZE 0x10000
#define FW_MAX_RETRY 100

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
	std::string lastConfName;
	std::string firmWareFileName;
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
		LoadFW = 0x02, // Загрузка ПО
		LoadCM = 0x03,  // Загрузка КМ
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
	std::string getFirmWareFileName() { return firmWareFileName; }
	
	//set
	void setZipLocation(const std::string & location) { zipLocation = location; }
	void setzipCompressionLevel(short level) { zipCompressionLevel = level; }
	void setParse(bool state) { parseEnabled = state;}
	void setZip(bool state) { zipEnabled = state; }
	void zetSaveToSameFolder(bool state) { saveToSameFolder = state; }
	void setCreateCompressedFile(bool state) { createCompressedFile = state; }
	void setFirmWareFileName(const std::string & fileName) { firmWareFileName = fileName; }

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
	bool TESTCHECK();
	//set current ftdi number
	void setFTDIdevice(int number);
	//set current ftdi number due to its serial number
	void setFTDIDevice(char * serialNumber);
	//load CM to the first FTDI device
	bool loadConfiguration();
	//load CM to the FTDI device due its handle
	bool loadConfiguration(FT_HANDLE ft_handle);
	//load CM to the FTDI device due its serial number
	bool loadConfiguration(unsigned int deviceNumber);
	//load FW to the first FTDI device
	bool loadFirmWare();
	//load FW to the FTDI device due its serial number
	bool loadFirmWare(FT_HANDLE ft_handle);
	//load FW to the FTDI device due its serial number
	bool loadFirmWare(unsigned int deviceNumber);
	//receive byte from ftdi
	char receiveByte(FT_HANDLE ft_handle);
	//send FW packet to ftdi device
	bool sendFW(FT_HANDLE ft_handle);
	//ccreate packet to load FW
	void createFWPacket(std::vector <unsigned char> &buffer);
	//rebooting first ftdi device
	void rebootDevice();
	//return number of FTDI devices connected to PC
	static unsigned int getDevicesCount();
	//set serialNumber of the FTDI device with number devideNum 
	static void getSerialNumber(int deviceNum, char* serialNumber);
	//set descr of the FTDI device with number devideNum 
	static void getDeviceDesrc(int deviceNum, char * descr);
	//return the first FTDI device handle
	FT_HANDLE getFirstDeviceHandle();
	//return FTDI handle by description
	FT_HANDLE getDeviceByDescription(const std::string description);
	//write data to the FTDI device
	FT_STATUS sendPacket(FT_HANDLE ftHandle, std::vector<unsigned char> &buffer, DWORD bytesToSend, LPDWORD lpdwBytesWritten);
	//return FDTI device handle by its number
	FT_HANDLE getDeviceHandle(unsigned int DeviceNumber);
	//create packet to send CM
	void createPacket(std::vector <unsigned char> &buffer);
	//return int value responce
	int readResponse(FT_HANDLE ft_handle, unsigned int size);
	//close the FTDI device
	FT_STATUS closeFTDI(FT_HANDLE ftHandle);
	//send one short command to the FTDDI device
	FT_STATUS sendShortCommand(FT_HANDLE ft_handle, Commands command);
	//set default FTDI settings
	bool setFTDISettings(FT_HANDLE ft_handle);
	//set FTDI settings
	bool setFTDISettings(FT_HANDLE ft_handle, unsigned long baudRate,
		unsigned char worldLength, unsigned char stopBits, unsigned char parity,
		unsigned long readTimeOut, unsigned long writeTimeOut);
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
