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
static TCHAR szTitle[] = _T("FW loader");
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
	configSL_,
	deviceSL_,
	openCMPB_,
	loadCMPB_,
	refreshDevicesPB_,
	confWayLE_,
	devicesDL_,
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
int fileSize(const std::string& fileName);
/*end of operations*/ 

/*events*/
void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
std::string getFileName(HWND hWnd, PCHAR lpstrFilter, PCHAR lpstrTitle);
void onLoadCMClicked(HWND hWnd);
void onRefreshDevicesPBClicked(HWND hWnd);
void onDeviceChanged();
/*end of events*/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	auto hWnd = initMainWindow(hInstance, hPrevInstance, lpCmdLine);

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
			onConfFileSelected(hWnd, confWayLE,_T("firmware (*.bin)\0*.bin\0All files(*.*)\0*.*\0"), _T("Выбор файла ПО"));
			break;
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
	/*static labels*/
	configSL = createWidget(static_label, _T("Firmware:"), hWnd, 10, 23, 100, 20, configSL_);
	deviceSL = createWidget(static_label, _T("Устройство:"), hWnd, 10, 60, 100, 20, deviceSL_);
	/*status bar*/
	stateSB = createWidget(status_bar, _T("Все готово к работе!"), hWnd, 10, 177, 470, 20, stateSB_);
	SetWindowText(stateSB, _T("Все готово к работе!"));
	/*line edits*/
	confWayLE = createWidget(line_edit, _T("D:\\commod.bin"), hWnd, 120, 20, 410, 25, confWayLE_);
	/*push buttons*/
	loadCMPB  = createWidget(push_button, _T("Загрузить"), hWnd, 450, 95, 100, 25, loadCMPB_);
	openCMPB  = createWidget(push_button, _T("..."), hWnd, 530, 20, 23, 25, openCMPB_);
	refreshDevicesPB = createWidget(push_button, _T("Обновить"), hWnd, 450, 57, 100, 25, refreshDevicesPB_);
	/*drop lists*/
	devicesDL = createWidget(drop_list, nullptr, hWnd, 120, 57, 320, 200, devicesDL_);
	onRefreshDevicesPBClicked(hWnd);
	/*progress bar*/
	hProgBar = createWidget(progress_bar, nullptr, hWnd, 10, 97, 430, 20, hProgBar_);
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

void onLoadCMClicked(HWND hWnd)
{
	setStatusBarText(L"Загрузка...");
	//if (currentDeviceNumber < 0)
	//{
	//	setStatusBarText(L"Не выбрано устройство!");
	//	return;
	//}
	auto szFileName = new char[MAX_PATH];
	GetWindowTextA(confWayLE, szFileName, MAX_PATH);
	std::string pathToCommodFile(szFileName);	
	delete[] szFileName;
	if (!isFileExists(pathToCommodFile)) {
		setStatusBarText(L"Не найден файл ПО.");
		return;
	}
	auto manager = new StrategyDeployment(pathToCommodFile);
	incrProgressBar(hWnd, 20);
	bool isOK= TRUE;
	manager->setFirmWareFileName(pathToCommodFile);
	manager->TESTCHECK();
	std::string resultLog;
	delete manager;
	incrProgressBar(hWnd, 30);
	isOK ? resultLog.append(" - ПО загружено успешно.") :
		resultLog.append(" -ПО не загружено.");
	
	SetWindowTextA(stateSB, resultLog.c_str());
}

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