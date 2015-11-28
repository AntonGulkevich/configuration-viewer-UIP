#ifndef STRATEGYDEPLOYMENT_H
#define STRATEGYDEPLOYMENT_H

#define EMULATION
#include <string>
#include <list>
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>

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
#define REWRITE_STATE 0x51A1DA1A
#define FW_WRITE_DONE_FLAG 0x2
#define FW_ERASE_DONE_FLAG 0x1
#define FW_ALL_DONE_FLAG 0x3

//end of define flags

#define DELIMITER "GENERAL"
#include "ftd2xx.h"
#include "crc32.h"
#include <vector>
#pragma comment(lib, "ftd2xx.lib") 

class StrategyDeployment 
{
private:
	std::string folderSaveName;
	std::string firmWareFileName;
	bool saveToSameFolder;
	std::list<std::string> logList;
	HWND hwnd;
	struct Fat {
		unsigned int address;
		unsigned int size;
	};
	unsigned int currentFTDIDevice;
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
	//constructor
	explicit StrategyDeployment();
	//get
	bool isSaveToSameFolder() const  { return saveToSameFolder; }
	std::string getFirmWareFileName() { return firmWareFileName; }	
	//set
	void zetSaveToSameFolder(bool state) { saveToSameFolder = state; }
	void setFirmWareFileName(const std::string & fileName) { firmWareFileName = fileName; }
	void setHwindow(HWND hwnd_) { hwnd = hwnd_;}
	//operations with files
	bool saveFile(const std::string fileName, unsigned char *buffer, long size, const std::string &param);
	bool saveFile(const std::string fileName, const std::string &buffer, long size, const std::string &param);
	bool saveFile(const std::string fileName, const std::vector<unsigned char> vecToSave);
	bool saveFile(const std::string fileName, const std::list<std::string> listToSave);
	bool isFileExists(const std::string& name) const{
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}
	long getFileSize(std::string filename) const{
		struct stat stat_buf;
		int rc = stat(filename.c_str(), &stat_buf);
		return rc == 0 ? stat_buf.st_size : -1;
	}
	void addIntToVect(int var, std::vector<unsigned char> &vector);
	void addShortToVect(short var, std::vector <unsigned char> &vector);
	
	/**********************ft_device*****************************/

	//set current FTDI device number
	void setFTDIdevice(int number);
	//set current FTDI device number due to its serial number
	void setFTDIDevice(char * serialNumber);
	//load FW to the first FTDI device
	bool loadFirmWare();
	//load FW to the FTDI device due its serial number
	bool loadFirmWare(FT_HANDLE ft_handle);
	//load FW to the FTDI device due its serial number
	bool loadFirmWare(unsigned int deviceNumber);
	//receive byte from FTDI device 
	char receiveByte(FT_HANDLE ft_handle);
	//send FW packet to FTDI device 
	bool sendFW(FT_HANDLE ft_handle);
	//rebooting the first FTDI device 
	void rebootDevice();
	//reading reply after loading fw
	int readReplyState(FT_HANDLE ft_handle);
	//return number of FTDI devices connected to PC
	static unsigned int getDevicesCount();
	//return number of FTDI devices starts with UBS-K string connected to PC
	static unsigned int getUBSKDevicesCount();
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
	//return int value responce
	int readResponse(FT_HANDLE ft_handle, unsigned int size);
	//close the FTDI device
	FT_STATUS closeFTDI(FT_HANDLE ftHandle);
	//send one short command to the FTDDI device
	bool sendShortCommand(FT_HANDLE ft_handle, Commands command);
	//set default FTDI settings
	bool setFTDISettings(FT_HANDLE ft_handle);
	//set FTDI settings
	bool setFTDISettings(FT_HANDLE ft_handle, unsigned long baudRate,
		unsigned char worldLength, unsigned char stopBits, unsigned char parity,
		unsigned long readTimeOut, unsigned long writeTimeOut);
	//log
	void saveLog();
	//console
	static void showState(double pos, int barWidth, double  currentProgress, double max);
	//destr
	~StrategyDeployment();	
};
#endif STRATEGYDEPLOYMENT_H
