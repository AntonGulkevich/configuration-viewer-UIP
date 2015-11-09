#include "StrategyDeployment .h"

StrategyDeployment::StrategyDeployment(const std::string commodFileName_) :
	commodFileName(commodFileName_)
{
	zipEnabled = false;
	parseEnabled = false;
	saveToSameFolder = true;
	zipCompressionLevel = 7;
	createCompressedFile = false;
	commodFileSize = 0;
	zipLocation = "";
	commodFile = nullptr;
}

bool StrategyDeployment::zip(const std::string& sourceFileName, const std::string zippedFileName)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	auto appParam = "7z a -tzip -mx" + std::to_string(zipCompressionLevel) + " \"" + zippedFileName + "\"" + " \"" + sourceFileName + "\"";
	if (!CreateProcessA(zipLocation.c_str(),
		_strdup(appParam.c_str()),
		nullptr,           // Process handle not inheritable
		nullptr,           // Thread handle not inheritable
		FALSE,			   // Set handle inheritance to FALSE
		CREATE_NO_WINDOW,  // Don't show 7zip log    
		nullptr,           // Use parent's environment block
		nullptr,           // Use parent's starting directory 
		&si,               // Pointer to STARTUPINFO structure
		&pi)               // Pointer to PROCESS_INFORMATION structure
		)
	{
		logList.push_back("CreateProcess failed. Error: " + GetLastError());
		return false;
	}

	// Wait until child process exits.
	WaitForSingleObject(pi.hProcess, INFINITE);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	logList.push_back("File: " + sourceFileName + ". Succsessully compressed to " + zippedFileName);
	return true;
}

bool StrategyDeployment::unzip(const std::string zippedFileName) const
{
	//no need
	return true;
}

bool StrategyDeployment::to_zip()
{
	return zip(commodFileName, commodFileName + ".zip");
}

bool StrategyDeployment::saveFile(const std::string fileName, unsigned char *buffer, long size, const std::string &param)
{
	FILE *file;
	if (fopen_s(&file, fileName.c_str(), param.c_str()) != 0)
	{
		logList.push_back("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()));
		return false;
	}
	if (!fwrite(buffer, sizeof(char), size, file))
	{
		logList.push_back("Unable to saveFile: " + fileName + ". Error: " + std::to_string(GetLastError()));
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
		logList.push_back("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()));
		return false;
	}
	if (!fwrite(cstr, sizeof(char), size, file))
	{
		logList.push_back("Unable to saveFile: " + fileName + ". Error: " + std::to_string(GetLastError()));
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
		logList.push_back("Unable to open file for write: " + fileName + ". Error: " + std::to_string(GetLastError()));
		return false;
	}
	for (auto it = listToSave.begin(); it != listToSave.end(); ++it)
	{
		fwrite((*it).c_str(), sizeof(char), (*it).size(), file);
	}
	fclose(file);
	return true;

}

bool StrategyDeployment::openfile(const std::string fileName)
{
	if (!isFileExists(fileName))
	{
		logList.push_back("File:" + fileName + " is not exist");
		return false;
	}
	if (fopen_s(&commodFile, fileName.c_str(), "r+b") != 0)
		logList.push_back("Can not open file: " + fileName + ". Error: " + std::to_string(GetLastError()));
	commodFileSize = getFileSize(fileName);
	return true;
}

bool StrategyDeployment::convert()
{
	//parsing file
	//parsing header
	if (!openfile(commodFileName)) {
		return false;
	}
	auto header = new char[HEADER_SIZE];
	fread_s(header, HEADER_SIZE, sizeof(char), HEADER_SIZE, commodFile);

	//parsing data
	int dataSize = commodFileSize - HEADER_SIZE - 7;
	auto dataToParse = new char[dataSize];
	fseek(commodFile, HEADER_SIZE + 7, SEEK_SET);
	fread_s(dataToParse, dataSize, sizeof(char), dataSize, commodFile);
	std::list <std::string> stringList;
	std::string dataString(dataToParse);
	std::string delimiter = DELIMITER;
	size_t pos;
	while ((pos = dataString.find(delimiter)) != std::string::npos)
	{
		std::string token = dataString.substr(0, pos);
		dataString.erase(0, pos + delimiter.length());
		stringList.push_back(token);
	}
	stringList.push_back(dataString);
	dataString = "";

	//stringList.pop_front();//the firts part is always zero
	auto address = HEADER_SIZE;
	std::list <Fat> headerList;
	for (auto it = stringList.begin(); it != stringList.end(); ++it)
	{
		auto temp = delimiter + *it;
		temp = temp.erase((temp.find("\r\nEND\r\n") + 7), temp.size());
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(=)[ ]{0,}"), std::string("="));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(==)[ ]{0,}"), std::string("=="));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(>)[ ]{0,}"), std::string(">"));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(<)[ ]{0,}"), std::string("<"));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(\\&\\&)[ ]{0,}"), std::string("&&"));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(\\|\\|)[ ]{0,}"), std::string("||"));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(\\})[ ]{0,}"), std::string("}"));
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(\\{)[ ]{0,}"), std::string("{"));
		temp = boost::regex_replace(temp, boost::regex("\t"), std::string(""));

		//deleting comments
		auto charsToDel = 0;
		auto start = 0;
		int tempCount = temp.size() - 1;
		for (auto j = 0; j<tempCount; ++j)
		{
			if (temp[j] == '/'&&temp[j + 1] == '/')
			{
				start = j;
				charsToDel = 0;
				while (temp[j++] != '\r')
					charsToDel++;

				temp.erase(start, charsToDel);
				j -= charsToDel;
				tempCount = tempCount - charsToDel;
			}
		}
		tempCount = temp.size() - 1;
		for (auto j = 0; j<tempCount; ++j)
		{
			if (temp[j] == ' '&&temp[j + 1] == ' ')
			{
				start = j;
				charsToDel = 0;
				while (temp[j++] == ' ')
					charsToDel++;

				temp.erase(start, charsToDel);
				j -= charsToDel;
				tempCount = tempCount - charsToDel;
			}
		}
		temp = boost::regex_replace(temp, boost::regex("(\r\n){2,}"), std::string("\r\n"));
		replaceAll(temp, "}\r\n", "}");
		replaceAll(temp, "{\r\n", "{");
		temp = boost::regex_replace(temp, boost::regex("[ ]{0,}(;\r\n)[ ]{0,}"), std::string(";"));
		/*text part must be multiply of 256: fill 0xFF*/
		tempCount = temp.size();
		int bytesToFill = HEADER_SIZE - tempCount%HEADER_SIZE;
		auto fillStr = new char[bytesToFill];
		memset(fillStr, 0xFF, bytesToFill);
		temp.append(fillStr, bytesToFill);

		/*calc header*/
		Fat tempFat;
		tempFat.address = address;
		tempFat.size = tempCount;
		address += temp.size();
		headerList.push_back(tempFat);
		/*end of cal header*/
		*it = temp;
		dataString += temp;
		//saveFile(commodFileName + std::to_string(testFilename++), temp, temp.size(), "wb");
		delete[] fillStr;
	}
	/*create new header for converted file*/
	std::vector<BYTE> headerNew;
	for (auto it = headerList.begin(); it != headerList.end(); ++it)
	{
		headerNew.push_back(static_cast<BYTE>(((*it).address & (0xFF << 8)) >> 8));
		headerNew.push_back(static_cast<BYTE>(((*it).address & (0xFF << 16)) >> 16));

		headerNew.push_back(static_cast<BYTE>(((*it).size & (0xFF << 0)) >> 0));
		headerNew.push_back(static_cast<BYTE>(((*it).size & (0xFF << 8)) >> 8));
	}
	for (size_t headerVecotrSize = headerNew.size(); headerVecotrSize != HEADER_SIZE; ++headerVecotrSize)
		headerNew.push_back(0xFF);
	//done creating new header

	bool isOk = saveFile(commodFileName + "_a", headerNew);
	isOk = saveFile(commodFileName + "_a", dataString, dataString.size(), "a+b");

	isOk ? logList.push_back("File: " + commodFileName + " sucsessfully converted to\nFile: " + commodFileName + "_a.") :
		logList.push_back("Error: can not convert file " + commodFileName);

	fclose(commodFile);
	if (createCompressedFile&&isFileExists(zipLocation))
		to_zip();
	delete[] header;
	delete[] dataToParse;
	return isOk;
}

bool StrategyDeployment::validateCurrentConfiguration()
{
	auto hInstDll = LoadLibrary(_T("win32dlib.dll"));
	if (hInstDll != nullptr)
		logList.push_back("Dll loaded successfully.");
	else {
		logList.push_back("Error: Failed to load dll\n");
		return false;
	}
	auto pDllGetFactory = reinterpret_cast<DLLGETFACTORY>(GetProcAddress(hInstDll, "returnFactory"));
	auto pMyFactory = (pDllGetFactory)();
	if (pMyFactory == nullptr)
		return false;
	auto drIoManager = pMyFactory->CreateDriversIoManager();
	auto drManager = pMyFactory->CreateDriverManager();
	std::string currentCommodFile = parseEnabled ? commodFileName + "_a" : commodFileName;
	drManager->setI2cCommodFileName(currentCommodFile);
	auto testWorkManager = pMyFactory->CreateWorkManager(drIoManager, drManager);
	testWorkManager->initCommod();
	logList.push_back("Configuration file validation: " + currentCommodFile);
	auto validConfig = testWorkManager->ValidateConfig();
	auto reg = testWorkManager->GetConfiguration()->GetRegister(REGISTER_ID::rNAME);
	lastConfName = std::string(reg->GetValueStr());
	delete drIoManager;
	delete drManager;
	delete testWorkManager;
	if (FreeLibrary(hInstDll))
		logList.push_back("Library successfully freed.");
	if (!validConfig) {
		logList.push_back("Invalid configuration.");
		return false;
	}
	return validConfig;
}

int StrategyDeployment::getCRC32Commod()
{
	if (commodFile != nullptr)
		fclose(commodFile);
	if (!openfile(parseEnabled ? commodFileName + "_a" : commodFileName))
		return 0;
	auto rawBuffer = new unsigned char[commodFileSize];
	fread_s(rawBuffer, commodFileSize, sizeof(char), commodFileSize, commodFile);
	fclose(commodFile);

	CRC32_n check;
	check.ProcessCRC(rawBuffer, commodFileSize);
	delete[] rawBuffer;
	return check.GetCRC32();
	//WARNING
	//this crc32 is not valid crc32 or crc32b due to current specification
	//CRC-32Q specification in aviation Aeronautical Information Exchange Model (AIXM) 0x814141AB
	//using polinom 0x8DD202EF
	//-------------------------------------------------------------------------------------------
	//valid calculation (crc32b specification) 
	/*boost::crc_32_type checksum_agent;
	checksum_agent.process_bytes(rawBuffer, commodFileSize);
	int a= checksum_agent.checksum();*/
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

bool StrategyDeployment::loadConfiguration()
{
	if (getDevicesCount() <= 0) {
		logList.push_back("No FTDI defices found!");
		return false;
	}
	logList.push_back("Connecting to the first FTDI device");
	return loadConfiguration(getFirstDeviceHandle());
}

bool StrategyDeployment::loadConfiguration(FT_HANDLE ft_handle)
{
	if (ft_handle == nullptr)
		return false;
	if (sendShortCommand(ft_handle, Commands::DiagnosticDisable) != FT_OK) {
		logList.push_back("Error: can't send command!");
		return false;
	}
	if (!setFTDISettings(ft_handle))
		return false;
	Sleep(1000);
	if( FT_Purge(ft_handle, FT_PURGE_RX )!= FT_OK)
	{
		logList.push_back("Error :FT_Purge receive	buffer.");
		return false;
	}
	/*send command load CM*/
	std::vector <unsigned char> buffer;
	createPacket(buffer);//creating packet
	unsigned long bytesSended;
	if (sendPacket(ft_handle, buffer, buffer.size(), &bytesSended) != FT_OK) {
		logList.push_back("Error: FT_Write.");
		return false;
	}
	/*read reply*/
	if (readResponse(ft_handle, 15) != OKREPLY)
	{
		logList.push_back("Error: configuration can not be load.");
		return false;
	}
	logList.push_back("Configuration loaded to the FTDI device.");
	/*reboot*/
	if (sendShortCommand(ft_handle, Commands::Reboot) != FT_OK) {
		logList.push_back("Error: cant send command!");
		return false;
	}
	logList.push_back("Rebooting the firts FTDI device.");
	FT_Close(ft_handle);
	return true;
}

bool StrategyDeployment::loadConfiguration(unsigned int deviceNumber)
{
	return loadConfiguration(getDeviceHandle(deviceNumber));
}

bool StrategyDeployment::loadFirmWare()
{
	if (getDevicesCount() <= 0) {
		logList.push_back("No FTDI defices found!");
		return false;
	}
	logList.push_back("Connecting to the first FTDI device");
	return loadFirmWare(getFirstDeviceHandle());
}

bool StrategyDeployment::loadFirmWare(FT_HANDLE ft_handle)
{
	if (ft_handle == nullptr)
		return false;
	if (!setFTDISettings(ft_handle))
		return false;
	if (sendShortCommand(ft_handle, DiagnosticDisable) != FT_OK) {
		logList.push_back("Error: can't send command!");
		return false;
	}
	if (sendShortCommand(ft_handle, LoadFW) != FT_OK) {
		logList.push_back("Error: can't send command!");
		return false;
	}
	if (readResponse(ft_handle, 15) != OKREPLY)
	{
		logList.push_back("Error: fw can not be load.");
		return false;
	}
	Sleep(500);
	if (FT_Purge(ft_handle, FT_PURGE_RX) != FT_OK)
	{
		logList.push_back("Error :FT_Purge receive	buffer.");
		return false;
	}
	Sleep(500);
	if (!sendFW(ft_handle))
		return false;
	logList.push_back("FW loaded to the FTDI device.");
	/*reboot*/
	if (sendShortCommand(ft_handle, Reboot) != FT_OK) {
		logList.push_back("Error: cant send command!");
		return false;
	}
	logList.push_back("Rebooting the first FTDI device.");
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
		logList.push_back("Error: FT_Read.");
		return NULL;
	}
	char response = byteRep[0];
	delete[] byteRep;
	return response;
}

bool StrategyDeployment::sendFW(FT_HANDLE ft_handle)
{
	/*send FW to ftdi device*/
	if (!isFileExists(firmWareFileName))
	{
		logList.push_back("File: " + firmWareFileName + " is not exist");
		return false;
	}
	FILE * fw_file;
	if (fopen_s(&fw_file, firmWareFileName.c_str(), "r+b") != 0)
		logList.push_back("Can not open file: " + firmWareFileName + ". Error: " + std::to_string(GetLastError()));
	auto fwFileSize = getFileSize(firmWareFileName);	
	short packetCount = std::ceil(static_cast<double>(fwFileSize)/static_cast<double>(FW_PACKET_SIZE));
	unsigned int lastPacketSize = fwFileSize - (packetCount - 1)*FW_PACKET_SIZE;
	CRC32_n crc32;
	for (auto packetNumber = 0; packetNumber < packetCount; ++packetNumber)
	{
		int packetSize = packetNumber != (packetCount - 1) ? FW_PACKET_SIZE : lastPacketSize;
		auto fwRawData = new unsigned char[packetSize];		
		fread_s(fwRawData, packetSize, sizeof(unsigned char), packetSize, fw_file);
		std::vector <unsigned char> buffer;
		crc32.Reset();
		crc32.ProcessCRC(fwRawData, packetSize);
		addIntToVect(FW_FLAG, buffer);
		addShortToVect(packetCount, buffer);
		addShortToVect(packetNumber, buffer);
		addIntToVect(crc32.GetCRC32(), buffer);
		buffer.insert(buffer.end(), fwRawData, fwRawData + packetSize);
		delete[] fwRawData;
		char reply = NULL;
		for (auto attempts_ = FW_MAX_RETRY; attempts_ != 0; --attempts_){
			unsigned long bytesSended;
			for (auto attempts = FW_MAX_RETRY; attempts != 0; --attempts){
				if (sendPacket(ft_handle, buffer, buffer.size(), &bytesSended) == FT_OK)
					break;
				logList.push_back("Error: FT_Write.");
				if (attempts == 1) {
					logList.push_back("Exceeded the maximum number of attempts.");
					fclose(fw_file);
					return false;
				}
			}			
			for (auto attempts = FW_MAX_RETRY; attempts != 0; --attempts){
				reply = receiveByte(ft_handle);
				if (reply != NULL) {
					break;
				}
				logList.push_back("Error: FW_No_Reply.");
				if (attempts == 1) {
					logList.push_back("Exceeded the maximum number of attempts.");
					fclose(fw_file);
					return false;
				}
			}
			if (reply != 0xAA || reply != 0x88 || reply != 0x99)
				break;
			if (attempts_ == 1) {
				logList.push_back("Exceeded the maximum number of attempts.");
				fclose(fw_file);
				return false;
			}
		} 
		if (reply != 0x77)			{
				logList.push_back("Error: firmware loading stopped with error byte received: " + std::to_string(reply));
				fclose(fw_file);
				return false;
			}
	}
	fclose(fw_file);
	return true;
}
bool StrategyDeployment::TESTCHECK()
{
	return true;
}

void StrategyDeployment::createFWPacket(std::vector<unsigned char>& buffer)
{

}

void StrategyDeployment::rebootDevice()
{
	if (getDevicesCount() <= 0) {
		logList.push_back("No FTDI defices found!");
		return;
	}
	logList.push_back("Connecting to the first FTDI device");
	FT_HANDLE ft_handle = getFirstDeviceHandle(); //open device
	if (ft_handle == nullptr)
		return;
	if (!setFTDISettings(ft_handle))
		return;
	if (sendShortCommand(ft_handle, Commands::Reboot) != FT_OK) {
		logList.push_back("Error: cant send command!");
		return;
	}
	FT_Close(ft_handle);
	logList.push_back("The first device rebooted.");
}

unsigned int StrategyDeployment::getDevicesCount()
{
	unsigned int ftdiDeviceCount;
	if (FT_ListDevices(&ftdiDeviceCount, nullptr, FT_LIST_NUMBER_ONLY) == FT_OK) {
		// FT_ListDevices OK, number of devices connected is in numDevs
		return ftdiDeviceCount;
	}
	else {
		//logList.push_back("Error: FT_ListDevices failed!");
		return -1;
	}
}

void StrategyDeployment::getSerialNumber(int deviceNum, char* serialNumber)
{
	FT_ListDevices(reinterpret_cast<PVOID>(deviceNum), serialNumber, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
}

void StrategyDeployment::getDeviceDesrc(int deviceNum, char* descr)
{

	FT_DEVICE_LIST_INFO_NODE *devInfo;
	DWORD numDevs;
	FT_STATUS ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (!ftStatus == FT_OK) {
		return;
	}
	if (numDevs > 0) {
		devInfo = static_cast<FT_DEVICE_LIST_INFO_NODE*>(malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs));
		ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
		if (ftStatus == FT_OK) {
			strcpy_s(descr, sizeof(devInfo[deviceNum].Description), devInfo[deviceNum].Description);
		}
	}
}

FT_HANDLE StrategyDeployment::getFirstDeviceHandle()
{
	return getDeviceHandle(0);

}

FT_HANDLE StrategyDeployment::getDeviceByDescription(const std::string description)
{
	if (getDevicesCount() == 0) {
		logList.push_back("No devices have been found.");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	auto ftStatus = FT_OpenEx(PVOID(description.c_str()), FT_OPEN_BY_DESCRIPTION, &ftHandle);
	if (ftStatus == FT_OK) {
		// FT_Open OK, use ftHandle to access device
		logList.push_back("The device: " + description + " opened.");
		return ftHandle;
	}
	else {
		// FT_Open failed
		logList.push_back("Error: The device: " + description + " can not be opened!");
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
		logList.push_back("No devices have been found.");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	if (FT_Open(DeviceNumber, &ftHandle) == FT_OK) {
		// FT_Open OK, use ftHandle to access device
		logList.push_back("Device opened.");
		return ftHandle;
	}
	else {
		// FT_Open failed
		logList.push_back("Error: the device can not be opened!");
		return nullptr;
	}
}

void StrategyDeployment::createPacket(std::vector <unsigned char> &buffer)
{
	int headerPlisFlag = STARTFLAG;
	short headerPlisLength; //plis logic demands decrement body size
	int bodyFlag = BODYFLAG;
	int commodsize;
	int commodCRC32 = getCRC32Commod();
	int endFlag = ENDFLAG;
	int forgottenFlag = FORGOTTENFLAG;

	if (!openfile(parseEnabled ? commodFileName + "_a" : commodFileName))
		return;
	auto rawBuffer = new unsigned char[commodFileSize];
	fread_s(rawBuffer, commodFileSize, sizeof(char), commodFileSize, commodFile);
	commodsize = commodFileSize + 8;
	headerPlisLength = commodFileSize + 5 * sizeof(int) - 1;
	fclose(commodFile);

	addIntToVect(headerPlisFlag, buffer);
	addShortToVect(headerPlisLength, buffer);
	addIntToVect(bodyFlag, buffer);
	addIntToVect(commodsize, buffer);
	addIntToVect(forgottenFlag, buffer);
	buffer.insert(buffer.end(), rawBuffer, rawBuffer + commodFileSize);
	addIntToVect(commodCRC32, buffer);
	addIntToVect(endFlag, buffer);
}

int StrategyDeployment::readResponse(FT_HANDLE ft_handle, unsigned int size)
{
	unsigned long bytesRead;
	unsigned char* byteRep = new unsigned char[size];
	if (FT_Read(ft_handle, byteRep, size, &bytesRead) != FT_OK) {
		logList.push_back("Error: FT_Read.");
		return -1;
	}
	int response = (byteRep[bytesRead -1] << 24) | (byteRep[bytesRead - 2] << 16)
		| (byteRep[bytesRead - 3] << 8) | byteRep[bytesRead - 4];
	delete[] byteRep;
	return response;
}

FT_STATUS StrategyDeployment::closeFTDI(FT_HANDLE ftHandle)
{
	return FT_Close(ftHandle);
}


FT_STATUS StrategyDeployment::sendShortCommand(FT_HANDLE ft_handle, Commands command)
{
	short size = 4 * (3 + 1) - 1;
	std::vector <unsigned char> vecData;
	addIntToVect(STARTFLAG, vecData);
	addShortToVect(size, vecData);
	addIntToVect(COMMANDFLAG, vecData);
	addIntToVect(1, vecData);

	vecData.push_back(0x5D);
	vecData.push_back(command);
	vecData.push_back(0x00);
	vecData.push_back(0xE4);

	addIntToVect(ENDFLAG, vecData);
	unsigned long bytesWritten;
	return sendPacket(ft_handle, vecData, vecData.size(), &bytesWritten);
}

bool StrategyDeployment::setFTDISettings(FT_HANDLE ft_handle)
{
	if (FT_SetBaudRate(ft_handle, 12000000) != FT_OK) {
		logList.push_back("Error: FT_SetBaudRate.");
		return false;
	}
	if (FT_SetDataCharacteristics(ft_handle, 8, 0, 0) != FT_OK) {
		logList.push_back("Error: FT_SetDataCharacteristics.");
		return false;
	}
	if (FT_SetTimeouts(ft_handle, 2500, 200) != FT_OK) {
		logList.push_back("Error: FT_SetTimeouts.");
		return false;
	}
	return true;
}

bool StrategyDeployment::setFTDISettings(FT_HANDLE ft_handle, unsigned long baudRate, 
	unsigned char worldLength, unsigned char stopBits, unsigned char parity, 
	unsigned long readTimeOut, unsigned long writeTimeOut)
{
	if (FT_SetBaudRate(ft_handle, baudRate) != FT_OK) {
		logList.push_back("Error: FT_SetBaudRate.");
		return false;
	}
	if (FT_SetDataCharacteristics(ft_handle, worldLength, stopBits, parity) != FT_OK) {
		logList.push_back("Error: FT_SetDataCharacteristics.");
		return false;
	}
	if (FT_SetTimeouts(ft_handle, readTimeOut, writeTimeOut) != FT_OK) {
		logList.push_back("Error: FT_SetTimeouts.");
		return false;
	}
	return true;
}

void StrategyDeployment::saveLog()
{
	saveFile("log.txt", logList);
}

void StrategyDeployment::showLog()
{
	for (auto it = logList.begin(); it != logList.end(); ++it)
		std::cout << *it << std::endl;
}

bool StrategyDeployment::execute()
{
	if (isParseEnabked()) convert();
	if (validateCurrentConfiguration()) {
		loadConfiguration();
		return true;
	}
	return false;
}

StrategyDeployment::~StrategyDeployment()
{
	if (commodFile != nullptr) fclose(commodFile);
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}


