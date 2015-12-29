#include "StrategyDeployment .h"
#include <iostream>

StrategyDeployment::StrategyDeployment()
{
	saveToSameFolder = true;
	currentFTDIDevice = -1;
	currentPackenNumber = 0;
	protocolVersion = 1;
	initProtocolVersion();
}
bool StrategyDeployment::saveFile(const std::string fileName, unsigned char *buffer, long size, const std::string &param)
{
	FILE *file;
	if (fopen_s(&file, fileName.c_str(), param.c_str()) != 0)
	{
		addToLog("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()) + "\r\n");
		return false;
	}
	if (!fwrite(buffer, sizeof(char), size, file))
	{
		addToLog("Unable to saveFile: " + fileName + ". Error: " + std::to_string(GetLastError()) + "\r\n");
		return false;
	}
	fclose(file);
	return true;
}

bool StrategyDeployment::saveFile(const std::string fileName, const std::string& buffer, long size, const std::string& param)
{

	char *cstr = new char[buffer.length() + 1];
	strcpy_s(cstr, buffer.length() + 1, buffer.c_str());

	FILE *file;
	if (fopen_s(&file, fileName.c_str(), param.c_str()) != 0)
	{
		addToLog("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()) + "\r\n");
		return false;
	}
	if (!fwrite(cstr, sizeof(char), size, file))
	{
		addToLog("Unable to saveFile: " + fileName + ". Error: " + std::to_string(GetLastError()) + "\r\n");
		return false;
	}
	fclose(file);
	delete[] cstr;
	return true;

}

bool StrategyDeployment::saveFile(const std::string fileName, std::vector<unsigned char> vecToSave)
{
	int size = vecToSave.size();
	unsigned char * buffer = new unsigned char[size];
	stdext::checked_array_iterator<unsigned char *> chkd_test_array(buffer, size);
	std::copy_n(vecToSave.begin(), size, chkd_test_array);
	bool processOk = saveFile(fileName, buffer, size, "w+b");
	delete[] buffer;
	return processOk;
}

bool StrategyDeployment::saveFile(const std::string fileName, const std::list<std::string> listToSave)
{
	FILE *file;
	if (fopen_s(&file, fileName.c_str(), "a+b") != 0)
	{
		addToLog("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()) + "\r\n");
		return false;
	}
	for (auto it = listToSave.begin(); it != listToSave.end(); ++it)
	{
		fwrite((*it).c_str(), sizeof(char), (*it).size(), file);
	}
	fclose(file);
	return true;

}

void StrategyDeployment::addIntToVect(int var, std::vector<unsigned char>& vector)
{
	vector.push_back(static_cast<unsigned char>((var >> 0) & 0xFF));
	vector.push_back(static_cast<unsigned char>((var >> 8) & 0xFF));
	vector.push_back(static_cast<unsigned char>((var >> 16) & 0xFF));
	vector.push_back(static_cast<unsigned char>((var >> 24) & 0xFF));
}

void StrategyDeployment::addShortToVect(short var, std::vector<unsigned char>& vector)
{
	vector.push_back(static_cast<unsigned char>((var >> 0) & 0xFF));
	vector.push_back(static_cast<unsigned char>((var >> 8) & 0xFF));
}

void StrategyDeployment::setFTDIdevice(int number)
{
	currentFTDIDevice = number;
}

void StrategyDeployment::setFTDIDevice(char* serialNumber)
{
	int devCount = getDevicesCount();
	for (int i = 0; i < devCount; ++i)
	{
		char * buffer = new char[64];
		getDeviceDesrc(i, buffer);
		if (strcmp(serialNumber, buffer))
			setFTDIdevice(i);
		delete[] buffer;
		return;
	}
}
bool StrategyDeployment::loadFirmWare()
{
	if (getUBSKDevicesCount()<= 0) {
		addToLog("No FTDI defices found!\r\n");
		return false;
	}
	addToLog("Connecting to the first FTDI device...\r\n");
	return loadFirmWare(getFirstDeviceHandle());
}

bool StrategyDeployment::loadFirmWare(FT_HANDLE ft_handle)
{
	if (ft_handle == nullptr)
		return false;
	if (!setFTDISettings(ft_handle))
		return false;
	if (!sendShortCommand(ft_handle, DiagnosticDisable))
		return false;
	if (!sendShortCommand(ft_handle, LoadFW))
		return false;
	if (readResponse(ft_handle, 15) != OKREPLY)
	{
		addToLog("Error: fw can not be load.\r\n");
		return false;
	}
	Sleep(500);
	if (FT_Purge(ft_handle, FT_PURGE_RX) != FT_OK)
	{
		addToLog("Error :FT_Purge receive	buffer.\r\n");
		return false;
	}
	Sleep(500);
	if (!sendFW(ft_handle))
		return false;
	addToLog("FW loaded to the FTDI device.\r\n");
	/*reboot*/
	sendShortCommand(ft_handle, Reboot);
	addToLog("Rebooting the first FTDI device.\r\n");
	FT_Close(ft_handle);
	return true;
}

bool StrategyDeployment::loadFirmWare(unsigned int deviceNumber)
{
	return loadFirmWare(getDeviceHandle(deviceNumber));
}

char StrategyDeployment::receiveByte(FT_HANDLE ft_handle)
{
	unsigned long bytesRead;
	char* byteRep = new char[1];
	if (FT_Read(ft_handle, byteRep, 1, &bytesRead) != FT_OK) {
		addToLog("Error: FT_Read.\r\n");
		return NULL;
	}
	char response = byteRep[0];
	delete[] byteRep;
	return response;
}

bool StrategyDeployment::sendFW(FT_HANDLE ft_handle)
{
	/*send FirmWare to the ftdi device with handle ft_handle*/
	if (!isFileExists(firmWareFileName))
	{
		addToLog("File: " + firmWareFileName + " is not exist!\r\n");
		return false;
	}
	auto fwFileSize = getFileSize(firmWareFileName);
	FILE * fw_file;
	if (fopen_s(&fw_file, firmWareFileName.c_str(), "r+b") != 0)
		addToLog("Can not open file: " + firmWareFileName + ". Error: " + std::to_string(GetLastError())+ "\r\n");
	
	short packetCount = std::ceil(static_cast<double>(fwFileSize) / static_cast<double>(FW_PACKET_SIZE));
	unsigned int lastPacketSize = fwFileSize - (packetCount - 1)*FW_PACKET_SIZE;
	CRC32_n crc32;
	/*console setup*/
	auto barWidth = 70;
	auto pos = 0.0;
	auto step = static_cast<double>(barWidth) / static_cast<double>(packetCount);
	std::cout << "Не выключайте питание устройства! Идет установка ПО. \nЭтап 1/3: записываем новую версию ПО.\r\n";
	for (auto packetNumber = 0; packetNumber < packetCount; ++packetNumber)
	{
		auto fwRawData = new unsigned char[FW_PACKET_SIZE];
		memset(fwRawData, '0xFF', FW_PACKET_SIZE);
		fread_s(fwRawData, FW_PACKET_SIZE, sizeof(unsigned char), FW_PACKET_SIZE, fw_file);
		std::vector <unsigned char> buffer;
		crc32.Reset();
		crc32.ProcessCRC(fwRawData, FW_PACKET_SIZE);
		addIntToVect(FW_FLAG, buffer);
		addShortToVect(packetCount, buffer);
		addShortToVect(packetNumber, buffer);
		addIntToVect(crc32.GetCRC32(), buffer);
		buffer.insert(buffer.end(), fwRawData, fwRawData + FW_PACKET_SIZE);
		delete[] fwRawData;
		char reply = NULL;
		for (auto attempts_ = FW_MAX_RETRY; attempts_ != 0; --attempts_) {
			unsigned long bytesSended;
			for (auto attempts = FW_MAX_RETRY; attempts != 0; --attempts) {
				if (sendPacket(ft_handle, buffer, buffer.size(), &bytesSended) == FT_OK)
					break;
				addToLog("Error: FT_Write. Retry " + std::to_string(attempts)+"\r\n");
				if (attempts == 1) {
					addToLog("Exceeded the maximum number of attempts.\r\n");
					fclose(fw_file);
					return false;
				}
			}
			for (auto attempts = FW_MAX_RETRY; attempts != 0; --attempts) {
				reply = receiveByte(ft_handle);
				if (reply != NULL)
					break;
				addToLog("Error: FW_No_Reply. Retry " + std::to_string(attempts));
				if (attempts == 1) {
					addToLog("Exceeded the maximum number of attempts. Error:" + std::to_string(reply)+"\r\n");
					fclose(fw_file);
					return false;
				}
			}
			if (reply != 0xAA || reply != 0x88 || reply != 0x99)
				break;
			if (attempts_ == 1) {
				addToLog("Exceeded the maximum number of attempts. Error:" + std::to_string(reply)+"\r\n");
				fclose(fw_file);
				return false;
			}
		}
		if (reply != 0x77) {
			addToLog("Error: firmware loading stopped with error byte received: " + std::to_string(reply)+"\r\n");
			fclose(fw_file);
			return false;
		}
		pos += step;
		showState(pos, barWidth, packetNumber, packetCount);
	}
	
	//erase
	pos = 0;
	std::cout << "\nЭтап 2/3: стираем предыдущую версию ПО.\r\n";
	int state;
	for (auto stage = 0; stage < packetCount - 1; ++stage)
	{
		pos += step;
		showState(pos, barWidth, stage, packetCount - 1);
		do
		{
			state = readReplyState(ft_handle);
			Sleep(100);
		} while (state == NULL);
	}
	//write
	addToLog("All erase stages completed successfully.\r\n");
	std::cout << "\nЭтап 3/3: осталось совсем немного.";
	char waitingStr[] = "-\\|/";
	for (auto stage = 0; stage < 2; ++stage)
	{
		int posw8 = 0;
		do
		{
			//std::cout << waitingStr[posw8 == 4 ? posw8 = 0 : posw8++] << "\b";
			std::cout << ".";
			state = readReplyState(ft_handle);
			Sleep(30);
		} while (state == NULL);
	}
	addToLog("Write completed.\r\n");
	fclose(fw_file);
	return true;
}

void StrategyDeployment::rebootDevice()
{
	if (getDevicesCount() <= 0) {
		addToLog("No FTDI defices found!\r\n");
		return;
	}
	addToLog("Connecting to the first FTDI device\r\n");
	auto ft_handle = getFirstDeviceHandle();
	if (ft_handle == nullptr)
		return;
	if (!setFTDISettings(ft_handle))
		return;
	if (!sendShortCommand(ft_handle, Commands::Reboot))
		return;
	FT_Close(ft_handle);
	addToLog("The first device rebooted.");
}

int StrategyDeployment::readReplyState(FT_HANDLE ft_handle)
{
	int size = 15;
	unsigned long bytesRead;
	unsigned char* byteRep = new unsigned char[size];
	if (FT_Read(ft_handle, byteRep, size, &bytesRead) != FT_OK) {
		addToLog("Error: FT_Read.\r\n");
		return -1;
	}
	if (bytesRead == 0)
		return NULL;
	int response = (byteRep[bytesRead - 1] << 24) | (byteRep[bytesRead - 2] << 16)
		| (byteRep[bytesRead - 3] << 8) | byteRep[bytesRead - 4];
	delete[] byteRep;
	return response;
}

unsigned int StrategyDeployment::getDevicesCount()
{
	unsigned int ftdiDeviceCount;
	if (FT_ListDevices(&ftdiDeviceCount, nullptr, FT_LIST_NUMBER_ONLY) == FT_OK) {
		return ftdiDeviceCount;
	}
	else {
		return -1;
	}
}

unsigned StrategyDeployment::getUBSKDevicesCount()
{
	unsigned int ubsCount = 0;
	int devCount = getDevicesCount();
	char *descr;
	for (int i = 0; i < devCount; ++i)
	{
		descr = new char[64];
		getDeviceDesrc(i, descr);
		if (!strncmp(descr, "UBS-K", 4)) ubsCount++;
	}
	return ubsCount;
}

void StrategyDeployment::getSerialNumber(int deviceNum, char* serialNumber)
{
	FT_ListDevices(reinterpret_cast<PVOID>(deviceNum), serialNumber, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
}

void StrategyDeployment::getDeviceDesrc(int deviceNum, char* descr)
{

	FT_DEVICE_LIST_INFO_NODE *devInfo;
	DWORD numDevs;
	if (FT_CreateDeviceInfoList(&numDevs) != FT_OK) {
		return;
	}
	if (numDevs > 0) {
		devInfo = static_cast<FT_DEVICE_LIST_INFO_NODE*>(malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs));
		if (FT_GetDeviceInfoList(devInfo, &numDevs) == FT_OK) {
			strcpy_s(descr, sizeof(devInfo[deviceNum].Description), devInfo[deviceNum].Description);
		}
	}
}

FT_HANDLE StrategyDeployment::getFirstDeviceHandle()
{
	int devCount = getDevicesCount();
	char *descr;
	for (int i = 0; i < devCount; ++i)
	{
		descr = new char[64];
		getDeviceDesrc(i, descr);
		if (!strncmp(descr, "UBS-K", 4)) return getDeviceHandle(i);
	}
	return nullptr;
}

FT_HANDLE StrategyDeployment::getDeviceByDescription(const std::string description)
{
	if (getDevicesCount() == 0) {
		addToLog("No devices have been found.\r\n");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	if (FT_OpenEx(PVOID(description.c_str()), FT_OPEN_BY_DESCRIPTION, &ftHandle) == FT_OK) {
		addToLog("The device: " + description + " opened.\r\n");
		return ftHandle;
	}
	else {
		addToLog("Error: The device: " + description + " can not be opened!\r\n");
		return nullptr;
	}
}

FT_STATUS StrategyDeployment::sendPacket(FT_HANDLE ftHandle, std::vector<unsigned char> &buffer, DWORD bytesToSend, LPDWORD lpdwBytesWritten)
{
	unsigned char * rawBuffer;
	rawBuffer = reinterpret_cast<unsigned char*>(buffer.data());
	return FT_Write(ftHandle, rawBuffer, bytesToSend, lpdwBytesWritten);
}

FT_HANDLE StrategyDeployment::getDeviceHandle(unsigned int DeviceNumber)
{
	if (getDevicesCount() == 0) {
		addToLog("No devices have been found.\r\n");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	if (FT_Open(DeviceNumber, &ftHandle) == FT_OK) {
		// FT_Open OK, use ftHandle to access device
		addToLog("Device opened.\r\n");
		return ftHandle;
	}
	else {
		// FT_Open failed
		addToLog("Error: the device can not be opened!\r\n");
		return nullptr;
	}
}

int StrategyDeployment::readResponse(FT_HANDLE ft_handle, unsigned int size)
{
	unsigned long bytesRead;
	unsigned char* byteRep = new unsigned char[size];
	if (FT_Read(ft_handle, byteRep, size, &bytesRead) != FT_OK) {
		addToLog("Error: FT_Read.\r\n");
		return -1;
	}
	if (bytesRead == 0)
		return -1;
	int response = (byteRep[bytesRead - 1] << 24) | (byteRep[bytesRead - 2] << 16)
		| (byteRep[bytesRead - 3] << 8) | byteRep[bytesRead - 4];
	delete[] byteRep;
	return response;
}

FT_STATUS StrategyDeployment::closeFTDI(FT_HANDLE ftHandle)
{
	return FT_Close(ftHandle);
}


bool StrategyDeployment::sendShortCommand(FT_HANDLE ft_handle, Commands command)
{
	short size = protocolVersion ? 23 : 15;
	std::vector <unsigned char> vecData;
	addIntToVect(STARTFLAG, vecData);
	addShortToVect(size, vecData);
	if (protocolVersion)
	{
		addIntToVect(PACKET_NUMBER_MARKER, vecData);
		addIntToVect(++currentPackenNumber, vecData);
	}
	addIntToVect(COMMANDFLAG, vecData);
	addIntToVect(1, vecData);

	vecData.push_back(0x5D);
	vecData.push_back(command);
	vecData.push_back(0x00);
	vecData.push_back(0xE4);
	if (protocolVersion)
	{
		int bufferSize = 20;
		unsigned char * buffer = new unsigned char[bufferSize];
		stdext::checked_array_iterator<unsigned char *> chkd_test_array(buffer, bufferSize);
		std::copy_n(vecData.begin() + 6, bufferSize, chkd_test_array);
		CRC32_n crc32n;
		crc32n.ProcessCRC(buffer, bufferSize);
		addIntToVect(crc32n.GetCRC32(), vecData);
		delete[] buffer;
	}
	else {
		addIntToVect(ENDFLAG, vecData);
	}

	unsigned long bytesWritten;
	if (sendPacket(ft_handle, vecData, vecData.size(), &bytesWritten) != FT_OK)
	{
		addToLog("Error: command can not be sended!\r\n");
		return false;
	}
	if (protocolVersion) {
		if (!readVerificationReply(ft_handle))
		{
			addToLog("Error: wrong responce to command " + std::to_string(command) + " !\r\n");
			return false;
		}
	}
	return true;
}

bool StrategyDeployment::setFTDISettings(FT_HANDLE ft_handle)
{
	if (FT_SetBaudRate(ft_handle, 12000000) != FT_OK) {
		addToLog("Error: FT_SetBaudRate.\r\n");
		return false;
	}
	if (FT_SetDataCharacteristics(ft_handle, 8, 0, 0) != FT_OK) {
		addToLog("Error: FT_SetDataCharacteristics.\r\n");
		return false;
	}
	if (FT_SetTimeouts(ft_handle, 2500, 200) != FT_OK) {
		addToLog("Error: FT_SetTimeouts.\r\n");
		return false;
	}
	return true;
}

bool StrategyDeployment::setFTDISettings(FT_HANDLE ft_handle, unsigned long baudRate,
	unsigned char worldLength, unsigned char stopBits, unsigned char parity,
	unsigned long readTimeOut, unsigned long writeTimeOut)
{
	if (FT_SetBaudRate(ft_handle, baudRate) != FT_OK) {
		addToLog("Error: FT_SetBaudRate.\r\n");
		return false;
	}
	if (FT_SetDataCharacteristics(ft_handle, worldLength, stopBits, parity) != FT_OK) {
		addToLog("Error: FT_SetDataCharacteristics.\r\n");
		return false;
	}
	if (FT_SetTimeouts(ft_handle, readTimeOut, writeTimeOut) != FT_OK) {
		addToLog("Error: FT_SetTimeouts.\r\n");
		return false;
	}
	return true;
}

int StrategyDeployment::getVersion()
{
	FT_HANDLE ft_handle = getFirstDeviceHandle();
	setFTDISettings(ft_handle);
	sendShortCommand(ft_handle, DiagnosticDisable);
	sendShortCommand(ft_handle, VersionRequest);
	int size = 31;
	unsigned long bytesRead;
	unsigned char* byteRep = new unsigned char[size];	
	if (FT_Read(ft_handle, byteRep, size, &bytesRead) != FT_OK) {
		addToLog("Error: FT_Read.\r\n");
		return -1;
	}
	std::vector<unsigned char> data(byteRep, byteRep + size);
	if (bytesRead == 0)
		return -1;
	int response = data[22];
	delete[] byteRep;
	FT_Close(ft_handle);
	return response;
}

void StrategyDeployment::initProtocolVersion()
{
	FT_HANDLE ft_handle = getFirstDeviceHandle();
	if (ft_handle!=nullptr)
	{
		setFTDISettings(ft_handle);
		sendShortCommand(ft_handle, DiagnosticDisable);
		readVerificationReply(ft_handle) ? protocolVersion = 1 : protocolVersion = 0;	
	}
	FT_Close(ft_handle);
}

bool StrategyDeployment::readVerificationReply(FT_HANDLE ft_handle)
{
	int size = 11;
	unsigned long bytesRead;
	unsigned char* byteRep = new unsigned char[size];
	if (FT_Read(ft_handle, byteRep, size, &bytesRead) != FT_OK) {
		addToLog("Error: FT_Read.\r\n");
		return -1;
	}
	if (bytesRead == 0)
		return -1;
	int response = (byteRep[bytesRead-1] << 24) | (byteRep[bytesRead - 2] << 16)
		| (byteRep[bytesRead - 3] << 8) | byteRep[bytesRead - 4];
	int marker = (byteRep[bytesRead - 5] << 24) | (byteRep[bytesRead - 6] << 16)
		| (byteRep[bytesRead - 7] << 8) | byteRep[bytesRead - 8];
	delete[] byteRep;
	if (marker!= PACKET_NUMBER_MARKER)
		return false;	
	return response == currentPackenNumber;
}

void StrategyDeployment::saveLog()
{
	saveFile("log.txt", logList);
}

void StrategyDeployment::addToLog(const std::string& str)
{
	time_t t = time(0);  
	struct tm * now = localtime(&t);
	std::string dateTime = std::to_string(now->tm_year + 1900);
	dateTime = dateTime + "-" + std::to_string(now->tm_mon + 1) + "-" + std::to_string(now->tm_mday) + " " +
		std::to_string(now->tm_hour) + ":" + std::to_string(now->tm_min) + ":" + std::to_string(now->tm_sec)+ " ";
	dateTime += str;
	logList.push_back(dateTime);
}

StrategyDeployment::~StrategyDeployment()
{
}
void  StrategyDeployment::showState(double pos, int barWidth, double  currentProgress, double max) {
	auto cp = static_cast<int>((currentProgress + 1) / max * 100);
	if (cp > 99) {
		std::cout << std::right << "[" << std::setw(barWidth + 1) << std::setfill('=') << "] "
			<< std::right << std::setw(3) << std::setprecision(4) << cp << "% \r";
	}
	else {
		std::cout << std::right << "[" << std::setw(ceil(pos)) << std::setfill('=') << ">" << std::setw(ceil(barWidth - pos)) << std::setfill(' ') << "] "
			<< std::right << std::setw(3) << std::setprecision(4) << cp << "% \r";
	}
}