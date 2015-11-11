#include <windows.h>
#include <string.h>
#include <commctrl.h>
#include <tchar.h>
#include "StrategyDeployment .h"

#ifdef UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#pragma comment (lib, "Comctl32.lib")

/*global vars*/
static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("CM loader");
/*end of global vars*/

/*controlls*/
HINSTANCE hInst;
/*check box*/
//HWND convertCB;
//HWND compressCB;
//HWND saveLogCB;
/*static label*/
HWND configSL;
//HWND compressorSL;
HWND deviceSL; 
/*status bar*/
HWND stateSB;
/*push button*/
HWND openCMPB;
//HWND openZipPB;
HWND loadCMPB;
HWND refreshDevicesPB;
/*line edit*/
HWND confWayLE;
//HWND zipWayLE;
/*drop list*/
HWND devicesDL;
/*progress bar*/
HWND hProgBar;
/*end of controlls*/

/*logic vars*/
int commodSize;
int currentDeviceNumber = -1;
/*end of logic vars*/
enum messageType
{
	errorMessage,
	warningMessage,
};
enum  controlls
{
	convertCB_,
	compressCB_,
	saveLogCB_,
	configSL_,
	compressorSL_,
	deviceSL_,
	openCMPB_,
	openZipPB_,
	loadCMPB_,
	refreshDevicesPB_,
	confWayLE_,
	zipWayLE_,
	devicesDL_,
	hBitmap_,
	hProgBar_,
	stateSB_ 

};

enum typeControlls
{
	push_button,
	check_box,
	static_label,
	line_edit,
	drop_list,
	progress_bar,
	status_bar
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*operations*/
void GetFileName(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void initControls(HWND hWnd);
HWND initMainWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine);
HWND createWidget(int type, wchar_t * caption, HWND hWnd, int x, int y, int w, int h, int index);
bool isFileExists(PCHAR name);
bool isFileExists(const std::string &fileName);
//void enableCompressorGroup(bool state);
void showMessageBox(HWND hWnd, LPCWSTR text, LPCWSTR title, messageType type);
void incrProgressBar(HWND hWnd, int step);
void setStatusBarText(LPCWSTR text);
std::string getZipLocation(HWND hWnd);
int fileSize(const std::string& fileName);
/*end of operations*/ 

/*events*/
void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
std::string getFileName(HWND hWnd, PCHAR lpstrFilter, PCHAR lpstrTitle);
void onLoadCMClicked(HWND hWnd);
void onRefreshDevicesPBClicked(HWND hWnd);
void onDeviceChanged();
//void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
//void onComvertCBClicked();
//void onCompressCBClicked();
//void onSaveLogCBClicked();
/*end of events*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	
	HWND hWnd = initMainWindow(hInstance, hPrevInstance, lpCmdLine);

	initControls(hWnd);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case devicesDL_:
			onDeviceChanged();
			break;
		case loadCMPB_:
			onLoadCMClicked(hWnd);
			break;
		case openCMPB_:
			onConfFileSelected(hWnd, confWayLE,_T("configuratoin files (*.bin)\0*.bin\0All files(*.*)\0*.*\0"), _T("Выбор файла конфигурации"));
			break;
		//case openZipPB_:
		//	onZipFileselected(hWnd, zipWayLE, _T("7z.exe(7z.exe)\0*.exe\0All files(*.*)\0*.*\0"), _T("Выбор архиватора"));
		//	break;
		//case compressCB_:
		//	onCompressCBClicked();
		//	break;	
		//case convertCB_:
		//	onComvertCBClicked();
		//	break;
		//case saveLogCB_:
		//	onSaveLogCBClicked();
		//	break;
		case refreshDevicesPB_:
			onRefreshDevicesPBClicked(hWnd);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	} 
	return 0;
}

void GetFileName(HWND hWnd, HWND lineEdit,  PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	EnableWindow(hWnd, false);
	TCHAR szFileName[1000] = { 0 };
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = hInst;
	ofn.lpstrFilter = lpstrFilter; 
	ofn.lpstrTitle = lpstrTitle; 
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	if (GetOpenFileName(&ofn))
		SetWindowText(lineEdit, szFileName);
	EnableWindow(hWnd, true);
	SetFocus(hWnd);
}

void initControls(HWND hWnd)
{	
	/*check boxes*/ 
	/*convertCB  = createWidget(check_box, _T("Сжатие конфигурации"), hWnd, 10, 90, 200, 16, convertCB_);
	compressCB = createWidget(check_box, _T("Создать архив"), hWnd, 210, 90, 130, 16, compressCB_);
	saveLogCB = createWidget(check_box, _T("Сохранить лог"), hWnd, 350, 90, 130, 16, saveLogCB_);*/
	/*end of check boxes*/

	/*static labels*/
	configSL = createWidget(static_label, _T("Конфигурация:"), hWnd, 10, 23, 100, 20, configSL_);
	deviceSL = createWidget(static_label, _T("Устройство:"), hWnd, 10, 60, 100, 20, deviceSL_);
	/*end of static labels*/

	/*status bar*/
	stateSB = createWidget(status_bar, _T("Все готово к работе!"), hWnd, 10, 177, 470, 20, stateSB_);
	SetWindowText(stateSB, _T("Все готово к работе!"));
	/*end of status bar*/

	/*line edits*/
	confWayLE = createWidget(line_edit, _T("D:\\commod.bin"), hWnd, 120, 20, 410, 25, confWayLE_);
	//zipWayLE = createWidget(line_edit, _T("D:\\Program Files (x86)\\7-Zip\\7z.exe"), hWnd, 120, 55, 410, 25, zipWayLE_);
	/*end of line edits*/

	/*push buttons*/
	loadCMPB  = createWidget(push_button, _T("Загрузить"), hWnd, 450, 95, 100, 25, loadCMPB_);
	//EnableWindow(loadCMPB, false);
	openCMPB  = createWidget(push_button, _T("..."), hWnd, 530, 20, 23, 25, openCMPB_);
	//openZipPB = createWidget(push_button, _T("..."), hWnd, 530, 55, 23, 25, openZipPB_);
	refreshDevicesPB = createWidget(push_button, _T("Обновить"), hWnd, 450, 57, 100, 25, refreshDevicesPB_);
	/*end of push buttons*/

	/*drop lists*/
	devicesDL = createWidget(drop_list, nullptr, hWnd, 120, 57, 320, 200, devicesDL_);
	onRefreshDevicesPBClicked(hWnd);
	/*end of drop lists*/

	/*progress bar*/
	hProgBar = createWidget(progress_bar, nullptr, hWnd, 10, 97, 430, 20, hProgBar_);
	/*end of progress bar*/
	//enableCompressorGroup(false);
}

HWND initMainWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	if (!RegisterClassEx(&wcex))
	{
		setStatusBarText(L"Call to RegisterClassEx failed!");
		return nullptr;
	}
	hInst = hInstance;
	auto hWnd = CreateWindow(szWindowClass, szTitle,
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, 590, 195, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		setStatusBarText(L"Call to CreateWindow failed!");
		return nullptr;
	}
	return hWnd;
}

HWND createWidget(int type, wchar_t* caption, HWND hWnd, int x, int y, int w, int h, int index)
{
	switch (type) {
	case push_button:
		return CreateWindow(WC_BUTTON, caption, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);
	case check_box:
		return CreateWindow(WC_BUTTON, caption, WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);
	case static_label:
		return CreateWindow(WC_STATIC, caption, WS_CHILD | WS_VISIBLE,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);
	case line_edit:
		return CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, caption, WS_VISIBLE | WS_CHILD | ES_LEFT,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);
	case drop_list:
		return CreateWindow(WC_COMBOBOX, nullptr, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);;
	case progress_bar:
		return CreateWindowEx(NULL, PROGRESS_CLASS, nullptr, WS_CHILD | WS_VISIBLE,
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);
	case status_bar:
		return CreateWindowEx(NULL, STATUSCLASSNAME, nullptr,WS_CHILD | WS_VISIBLE, 
			x, y, w, h, hWnd, reinterpret_cast<HMENU>(index), hInst, nullptr);

	}
	return nullptr;
}

void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}

std::string getFileName(HWND hWnd, PCHAR lpstrFilter, PCHAR lpstrTitle)
{
	EnableWindow(hWnd, false);
	CHAR szFileName[1000] = { 0 };
	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = hInst;
	ofn.lpstrFilter = lpstrFilter;
	ofn.lpstrTitle = lpstrTitle;
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	std::string fileName;
	if (GetOpenFileNameA(&ofn))
		fileName.append(szFileName);
	EnableWindow(hWnd, true);
	SetFocus(hWnd);
	return fileName;
}

//void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
//{
//	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
//}

//void onComvertCBClicked()
//{
//	parseEnabled = (SendMessage(convertCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
//}

//void enableCompressorGroup(bool state)
//{
//	compressEnabled = state;
//	EnableWindow(zipWayLE, state);
//	EnableWindow(compressorSL, state);
//	EnableWindow(openZipPB, state);
//}

void showMessageBox(HWND hWnd, LPCWSTR text, LPCWSTR title, messageType type)
{
	switch(type)
	{
	case errorMessage:
		MessageBox(hWnd, text, title , MB_OK | MB_ICONERROR);
		break;
	case warningMessage:
		MessageBox(hWnd, text, title, MB_OK | MB_ICONEXCLAMATION);
		break;
	}
}

void incrProgressBar(HWND hWnd, int step)
{
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBS_SMOOTH, 0, 0);
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_SETSTEP, step, 0);
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_STEPIT, 0, 0);
}

void setStatusBarText(LPCWSTR text)
{
	SetWindowText(stateSB, text);
}

std::string getZipLocation(HWND hWnd)
{
	CHAR szNativeProgramFilesFolder[MAX_PATH];
	ExpandEnvironmentStringsA("%ProgramW6432%",
		szNativeProgramFilesFolder,
		ARRAYSIZE(szNativeProgramFilesFolder));

	std::string fullway(szNativeProgramFilesFolder);
	fullway.append("\\7 - Zip\\7z.exe");
	if (!isFileExists(fullway))
		fullway = getFileName(hWnd, "7z.exe(7z.exe)\0*.exe\0All files(*.*)\0*.*\0", "Выбор архиватора");
	return fullway;

}

void onLoadCMClicked(HWND hWnd)
{
	setStatusBarText(L"Загрузка...");
	if (currentDeviceNumber < 0)
	{
		setStatusBarText(L"Не выбрано устройство!");
		return;
	}

	auto szFileName = new char[MAX_PATH];
	GetWindowTextA(confWayLE, szFileName, MAX_PATH);
	std::string pathToCommodFile(szFileName);	
	delete[] szFileName;

	if (!isFileExists(pathToCommodFile)) {
		setStatusBarText(L"Не найден файл конфигурации");
		return;
	}
	auto manager = new StrategyDeployment(pathToCommodFile);
	incrProgressBar(hWnd, 20);
	bool isOK;
	if (fileSize(pathToCommodFile) > 0x10000)
	{
		manager->setZip(true);
		manager->setCreateCompressedFile(true);
		manager->setZipLocation(getZipLocation(hWnd));
		manager->setParse(true);
		manager->setzipCompressionLevel(7);
		isOK = manager->convert();
	}
	
	incrProgressBar(hWnd, 30);
	isOK = manager->validateCurrentConfiguration();	
	incrProgressBar(hWnd, 20);
	isOK = manager->loadConfiguration(currentDeviceNumber);
	manager->saveLog();
	auto resultLog(manager->getLastConfName());
	delete manager;
	incrProgressBar(hWnd, 30);
	isOK ? resultLog.append(" - конфигурация загружена успешно.") :
		resultLog.append(" -конфигурация не загружена.");
	
	SetWindowTextA(stateSB, resultLog.c_str());
}

//void onCompressCBClicked()
//{
//	enableCompressorGroup(SendMessage(compressCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
//}

//void onSaveLogCBClicked()
//{
//	saveLog = (SendMessage(saveLogCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
//}

void onRefreshDevicesPBClicked(HWND hWnd)
{
	SendMessage(devicesDL, CB_RESETCONTENT, 0, 0);
	unsigned int devicesCount = StrategyDeployment::getDevicesCount();
	if (devicesCount <= 0)
	{
		setStatusBarText(L"Не обнаружено FTDI устройств.");
		currentDeviceNumber = -1;
		return;
	}
	currentDeviceNumber = 0;

	for (int i = 0; i < devicesCount; ++i)
	{
		char *buffer = new char[64];
		char *serialNumber = new char[64];
		StrategyDeployment::getSerialNumber(i, serialNumber);
		StrategyDeployment::getDeviceDesrc(i, buffer);
		strcat_s(buffer, 64, " Serial num: ");
		strcat_s(buffer, 64, serialNumber);
		SendMessageA(devicesDL, CB_ADDSTRING, static_cast<WPARAM>(0), reinterpret_cast<LPARAM>(buffer));
		SendMessage(devicesDL, CB_SETCURSEL, static_cast<WPARAM>(0), 0);

		delete[] serialNumber;
		delete[] buffer;
	}

}

void onDeviceChanged()
{
	currentDeviceNumber = SendMessage(devicesDL, CB_GETCURSEL, 0, 0);
}

bool isFileExists(PCHAR name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
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