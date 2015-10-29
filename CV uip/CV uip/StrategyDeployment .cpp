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
	auto appParam = "7z a -tzip -mx" + std::to_string(zipCompressionLevel) + " \"" + zippedFileName +"\"" + " \"" + sourceFileName + "\"";
	if (!CreateProcessA(zipLocation.c_str(),
		_strdup(appParam.c_str()),
		nullptr,           // Process handle not inheritable
		nullptr,           // Thread handle not inheritable
		FALSE,			   // Set handle inheritance to FALSE
		0,  // Don't show 7zip log    
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
	if (fopen_s(&file, fileName.c_str(), param.c_str())!=0)
	{
		logList.push_back("Unable to open file for write: " + fileName + ". Error: "+ std::to_string(GetLastError()));
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

bool StrategyDeployment::saveFile(const std::string fileName,  std::vector<unsigned char> vecToSave)
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
	if (fopen_s(&commodFile, fileName.c_str(), "r+b")!=0)
		logList.push_back("Can not open file: " + fileName + ". Error: " + std::to_string(GetLastError()));
	commodFileSize = getFileSize(fileName);
	return true;
}

bool StrategyDeployment::convert()
{
	//parsing file
	//parsing header
	if (!openfile(commodFileName))	{
		return false;
	}
	auto header = new char[HEADER_SIZE];
	fread_s(header, HEADER_SIZE, sizeof(char), HEADER_SIZE, commodFile);

	//parsing data
	int dataSize = commodFileSize - HEADER_SIZE;
	auto dataToParse = new char[dataSize];
	fseek(commodFile, HEADER_SIZE, SEEK_SET);
	fread_s(dataToParse, dataSize, sizeof(char), dataSize, commodFile);
	std::list <std::string> stringList;
	std::string dataString(dataToParse);
	std::string delimiter = DELIMITER;
	size_t pos = 0;
	std::string token;
	while ((pos = dataString.find(delimiter)) != std::string::npos) 
	{
		token = dataString.substr(0, pos);
		stringList.push_back(token);
		dataString.erase(0, pos + delimiter.length());
	}
	dataString = "";
	stringList.push_back(token);
	stringList.pop_front();//the firts part is always zero
	auto address = HEADER_SIZE;
	std::list <Fat> headerList;
	for (auto it = stringList.begin(); it != stringList.end(); ++it)
	{
		auto temp = DELIMITER + *it;
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
		auto fillStr = new char [bytesToFill] ;
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
		dataString += temp ;
		delete[] fillStr;
	}
	/*create new header for converted file*/
	std::vector<BYTE> headerNew;
	auto start = 0;
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
	std::string resultStr;
	resultStr.append(dataString);
	isOk=isOk&saveFile(commodFileName + "_a", reinterpret_cast<unsigned char*>(_strdup(resultStr.c_str())), resultStr.size(), "a+b");

	isOk ? logList.push_back("File: " + commodFileName + " sucsessfully converted to\nFile: " + commodFileName + "_a.") :
		logList.push_back("Error: can not convert file " + commodFileName);

	fclose(commodFile);
	if (createCompressedFile)
		to_zip();
	delete[] header;
	delete[] dataToParse;
	return isOk;
}

bool StrategyDeployment::validateCurrentConfiguration()
{
	auto hInstDll = LoadLibrary(_T("win32dlib.dll"));
	if (hInstDll != nullptr) {
		logList.push_back("Dll loaded successfully.");
	}
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
	drManager->setI2cCommodFileName(commodFileName);
	auto testWorkManager = pMyFactory->CreateWorkManager(drIoManager, drManager);
	testWorkManager->initCommod();
	logList.push_back("Configuration file validation: " + commodFileName);
	auto validConfig = testWorkManager->ValidateConfig();
	delete drIoManager;
	delete drManager;
	delete testWorkManager;
	//better to free library, be aware of memory leak. (automatically free library when there is no process using it)
	if (FreeLibrary(hInstDll))
		logList.push_back("Library successfully freed.");
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

bool StrategyDeployment::loadConfiguration()
{
	//FT_DEVICE_LIST_INFO_NODE deviceInfo;
	if (getDevicesCount() <= 0)	{
		logList.push_back("No FTDI defices found!");
		return false;
	}
	logList.push_back("Connecting to the first FTDI device");
	FT_HANDLE ft_handle = getFirstDeviceHandle(); //open device
	if (ft_handle == nullptr)
		return false;
	if (!setFTDISettings(ft_handle))
		return false;
	if (sendCommand(ft_handle, Commands::LoadCM) != FT_OK){
		logList.push_back("Error: cant send command!");
		return false;
	}
	/*read reply*/
	if (readResponse(ft_handle, RESPONSESIZE) != OKREPLY)
	{
		logList.push_back("Error: configuration can not be load.");
		return false;
	}
	logList.push_back("Configuration loaded to the first FTDI device.");
	/*end of reply*/

	/*reboot*/
	if (sendCommand(ft_handle, Commands::Reboot) != FT_OK) {
		logList.push_back("Error: cant send command!");
		return false;
	}
	logList.push_back("Rebooting the firts FTDI device.");
	/*end of reboot*/
	FT_Close(ft_handle);
	return true;
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
	if (sendCommand(ft_handle, Commands::Reboot) != FT_OK) {
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

void StrategyDeployment::getSerialNumber(int devideNum, char* serialNumber)
{
	 FT_ListDevices(reinterpret_cast<PVOID>(devideNum), serialNumber, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
}

FT_HANDLE StrategyDeployment::getFirstDeviceHandle()
{
	if (getDevicesCount() == 0) {
		logList.push_back("No devices have been found.");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	if (FT_Open(0, &ftHandle) == FT_OK) {
		// FT_Open OK, use ftHandle to access device
		logList.push_back("The first device opened.");
		return ftHandle;
	}
	else {
		// FT_Open failed
		logList.push_back("Error: the first device can not be opened!");
		return nullptr;
	}

}

FT_HANDLE StrategyDeployment::getDeviceByDescription(const std::string description)
{
	if (getDevicesCount() == 0) {
		logList.push_back("No devices have been found.");
		return nullptr;
	}
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;

	ftStatus = FT_OpenEx(PVOID(description.c_str()), FT_OPEN_BY_DESCRIPTION, &ftHandle);
	if (ftStatus == FT_OK) {
		// FT_Open OK, use ftHandle to access device
		logList.push_back("The device: "+ description +" opened.");
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
	bool res = FT_Write(ftHandle, rawBuffer, bytesToSend, lpdwBytesWritten);
	return res;
}

FT_STATUS StrategyDeployment::sendCommand(FT_HANDLE ftHandle, Commands command)
{
	switch (command)
	{
	case 1: //VersionRequest
		return versionRequest();
	case 2: //LoadFW
		return loadFW();
	case 3: //LoadCM
		return loadCM(ftHandle);
	case 4: //SaveCM
		return saveCM();
	case 5: //DiagnosticEnable
		return diagnosticEnable();
	case 6: //DiagnosticDisable
		return diagnosticDisable();
	case 7: //Configuration
		return configuration();
	case 8: //GetChannelSettings
		return getChannelSettings();
	case 9: //GetFails
		return getFails();
	case 10: //GetDynamicInfo
		return getDynamicInfo();
	case 11: //Reboot
		return reboot(ftHandle);
	case 12: //StartCalibrate
		return startCalibrate();
	case 13: //GetCalibrates
		return getCalibrates();
	case 14: //SaveSettings
		return saveSettings();
	default:
		logList.push_back("Invalid command!");
		return -1;
	}

}

void StrategyDeployment::createPacket (std::vector <unsigned char> &buffer)
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

int StrategyDeployment::readResponse(FT_HANDLE ft_handle, unsigned long bytesToRead)
{
	unsigned long bytesReturned;
	unsigned char* byteRep = new unsigned char [bytesToRead];
	if (FT_Read(ft_handle, byteRep, bytesToRead, &bytesReturned) != FT_OK) {
		logList.push_back("Error: FT_Read.");
		return -1;
	}
	int response  = (byteRep[14] << 24) | (byteRep[13] << 16) | (byteRep[12] << 8) | byteRep[11];
	delete[] byteRep;
	return response;
}

FT_STATUS StrategyDeployment::closeFTDI(FT_HANDLE ftHandle)
{
	return FT_Close(ftHandle);
}

FT_STATUS StrategyDeployment::loadCM(FT_HANDLE ft_handle)
{
	std::vector <unsigned char> buffer;
	createPacket(buffer);//creating packet
	unsigned long bytesSended;	
	if (sendPacket(ft_handle, buffer, buffer.size(), &bytesSended)!= FT_OK) {
		logList.push_back("Error: FT_Write.");
		return -1;
	}
	return FT_OK;
}

FT_STATUS StrategyDeployment::reboot(FT_HANDLE ft_handle)
{
	short size = 4 * (3 + 1) - 1;
	std::vector <unsigned char> vecData;
	addIntToVect(STARTFLAG, vecData);	
	addShortToVect(size, vecData);
	addIntToVect(COMMANDFLAG, vecData);
	addIntToVect(1, vecData);	

	vecData.push_back(0x5D);
	vecData.push_back(0x0B);
	vecData.push_back(0x00);
	vecData.push_back(0xE4);

	addIntToVect(ENDFLAG, vecData);
	unsigned long bytesWritten;
	return sendPacket(ft_handle, vecData, vecData.size(), &bytesWritten);
}

FT_STATUS StrategyDeployment::versionRequest()
{
	return 0;
}

FT_STATUS StrategyDeployment::loadFW()
{
	return 0;
}

FT_STATUS StrategyDeployment::saveCM()
{
	return 0;
}

FT_STATUS StrategyDeployment::diagnosticEnable()
{
	return 0;
}

FT_STATUS StrategyDeployment::diagnosticDisable()
{
	return 0;
}

FT_STATUS StrategyDeployment::configuration()
{
	return 0;
}

FT_STATUS StrategyDeployment::getChannelSettings()
{
	return 0;
}

FT_STATUS StrategyDeployment::getFails()
{
	return 0;;
}

FT_STATUS StrategyDeployment::getDynamicInfo()
{
	return 0;
}

FT_STATUS StrategyDeployment::startCalibrate()
{
	return 0;
}

FT_STATUS StrategyDeployment::getCalibrates()
{
	return 0;
}

FT_STATUS StrategyDeployment::saveSettings()
{
	return 0;
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
	if (FT_SetTimeouts(ft_handle, 5000, 200) != FT_OK) {
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
	if (validateCurrentConfiguration()) {
		if (isParseEnabked()) convert();
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