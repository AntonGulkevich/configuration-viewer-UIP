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


/*global vars*/
static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("CM loader");
/*end of global vars*/

/*controlls*/
HINSTANCE hInst;
/*check box*/
HWND convertCB;
HWND compressCB;
HWND saveLogCB;
/*static label*/
HWND configSL;
HWND compressorSL;
HWND deviceSL;
/*push button*/
HWND openCMPB;
HWND openZipPB;
HWND loadCMPB;
HWND refreshDevicesPB;
/*line edit*/
HWND confWayLE;
HWND zipWayLE;
/*drop list*/
HWND devicesDL;
/*images & icons*/
HBITMAP hBitmap = nullptr;
/*progress bar*/
HWND hProgBar;
/*end of controlls*/

/*logic vars*/
bool parseEnabled = false;
bool compressEnabled = false;
bool saveLog = false;
/*end of logic vars*/

enum  controlls
{
	convertCB_ = 1,
	compressCB_ = 2,
	saveLogCB_ = 3,
	configSL_ = 4 ,
	compressorSL_ = 5,
	deviceSL_ = 6,
	openCMPB_ = 7,
	openZipPB_ = 8,
	loadCMPB_ = 9,
	refreshDevicesPB_ = 10,
	confWayLE_ = 11,
	zipWayLE_ = 12,
	devicesDL_ = 13,
	hBitmap_ = 14,
	hProgBar_ = 15,

};

enum typeControlls
{
	push_button,
	check_box,
	static_label,
	line_edit,
	drop_list,
	progress_bar
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*operations*/
void GetFileName(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void initControls(HWND hWnd);
HWND initMainWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine);
HWND createWidget(int type, wchar_t * caption, HWND hWnd, int x, int y, int w, int h, int index);
bool isFileExists(PCHAR name);
void enableCompressorGroup(bool state);
/*end of operations*/

/*events*/
void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void onComvertCBClicked();
void onLoadCMClicked(HWND hWnd);
void onCompressCBClicked();
void onSaveLogCBClicked();
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
	//case WM_PAINT:
		//PAINTSTRUCT 	ps;
		//HDC 			hdc;
		//BITMAP 			bitmap;
		//HDC 			hdcMem;
		//HGDIOBJ 		oldBitmap;

		//hdc = BeginPaint(hWnd, &ps);

		//hdcMem = CreateCompatibleDC(hdc);
		//oldBitmap = SelectObject(hdcMem, hBitmap);

		//GetObject(hBitmap, sizeof(bitmap), &bitmap);
		//BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

		//SelectObject(hdcMem, oldBitmap);
		//DeleteDC(hdcMem);

		//EndPaint(hWnd, &ps);
		//break;

	//case WM_SETFONT:
	//	SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM >(hFont), TRUE);
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case loadCMPB_: 
			/*SendMessage(GetDlgItem(hWnd, hProgBar_), PBS_SMOOTH, 0, 0);
			SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_SETSTEP, 100, 0);
			SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_STEPIT, 0, 0);*/
			onLoadCMClicked(hWnd);
			break;
		case openCMPB_:
			onConfFileSelected(hWnd, confWayLE,_T("configuratoin files (*.bin)\0*.bin\0All files(*.*)\0*.*\0"), _T("Выбор файла конфигурации"));
			break;
		case openZipPB_:
			onZipFileselected(hWnd, zipWayLE, _T("7z.exe(7z.exe)\0*.exe\0All files(*.*)\0*.*\0"), _T("Выбор архиватора"));
			break;
		case compressCB_:
			onCompressCBClicked();
			break;	
		case convertCB_:
			onComvertCBClicked();
			break;
		case saveLogCB_:
			onSaveLogCBClicked();
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
	convertCB  = createWidget(check_box, _T("Сжатие конфигурации"), hWnd, 10, 90, 200, 16, convertCB_);
	compressCB = createWidget(check_box, _T("Создать архив"), hWnd, 210, 90, 130, 16, compressCB_);
	saveLogCB = createWidget(check_box, _T("Сохранить лог"), hWnd, 350, 90, 130, 16, saveLogCB_);
	/*end of check boxes*/

	/*static labels*/
	configSL = createWidget(static_label, _T("Конфигурация:"), hWnd, 10, 23, 100, 20, configSL_);
	compressorSL = createWidget(static_label, _T("Архиватор:"), hWnd, 10, 56, 100, 20, compressorSL_);
	deviceSL = createWidget(static_label, _T("Устройство:"), hWnd, 10, 123, 100, 20, deviceSL_);
	/*end of static labels*/

	/*line edits*/
	confWayLE = createWidget(line_edit, _T("D:\\commod.bin"), hWnd, 120, 20, 410, 25, confWayLE_);
	zipWayLE = createWidget(line_edit, _T("D:\\Program Files (x86)\\7-Zip\\7z.exe"), hWnd, 120, 55, 410, 25, zipWayLE_);
	/*end of line edits*/

	/*push buttons*/
	loadCMPB  = createWidget(push_button, _T("Загрузить"), hWnd, 450, 155, 100, 25, loadCMPB_);
	//EnableWindow(loadCMPB, false);
	openCMPB  = createWidget(push_button, _T("..."), hWnd, 530, 20, 23, 25, openCMPB_);
	openZipPB = createWidget(push_button, _T("..."), hWnd, 530, 55, 23, 25, openZipPB_);
	refreshDevicesPB = createWidget(push_button, _T("Обновить"), hWnd, 450, 119, 100, 25, refreshDevicesPB_);
	/*end of push buttons*/

	/*drop lists*/
	devicesDL = createWidget(drop_list, nullptr, hWnd, 120, 120, 320, 200, devicesDL_);
	/*end of drop lists*/

	/*images and icons*/
	hBitmap = static_cast<HBITMAP>(LoadImage(hInst, _T("D:\\GIT\\configuration-viewer-UIP\\CV uip\\CV uip\\logo-top.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	/*end of images and icons*/

	/*progress bar*/
	hProgBar = createWidget(progress_bar, nullptr, hWnd, 10, 157, 430, 20, hProgBar_);
	/*end of progress bar*/

	enableCompressorGroup(false);
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
		MessageBox(nullptr,	_T("Call to RegisterClassEx failed!"),szTitle,	NULL);
		return nullptr;
	}
	hInst = hInstance;
	auto hWnd = CreateWindow(szWindowClass, szTitle,
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, 590, 230, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(nullptr,	_T("Call to CreateWindow failed!"),	szTitle,NULL);
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
	}
	return nullptr;
}

void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}

void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}

void onComvertCBClicked()
{
	parseEnabled = (SendMessage(convertCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void enableCompressorGroup(bool state)
{
	compressEnabled = state;
	EnableWindow(zipWayLE, state);
	EnableWindow(compressorSL, state);
	EnableWindow(openZipPB, state);
}

void onLoadCMClicked(HWND hWnd)
{
	char * szFileName = new char[1000];
	GetWindowTextA(confWayLE, szFileName, 1000);
	std::string pathToCommodFile(szFileName);
	delete[] szFileName;

	char * sevenZipFileName = new char[1000];
	GetWindowTextA(zipWayLE, sevenZipFileName, 1000);
	std::string pathTo7Zip(sevenZipFileName);
	delete[] sevenZipFileName;

	StrategyDeployment  *manager = new StrategyDeployment(pathToCommodFile);
	manager->setZip(compressEnabled);
	manager->setZipLocation(pathTo7Zip);
	manager->setCreateCompressedFile(compressEnabled);
	manager->setParse(parseEnabled);
	manager->setzipCompressionLevel(7);

	manager->convert();
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBS_SMOOTH, 0, 0);
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_SETSTEP, 20, 0);
	SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_STEPIT, 0, 0);
	manager->validateCurrentConfiguration();
	//SendMessage(GetDlgItem(hWnd, hProgBar_), PBS_SMOOTH, 0, 0);
	//SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_SETSTEP, 100, 0);
	//SendMessage(GetDlgItem(hWnd, hProgBar_), PBM_STEPIT, 0, 0);

	if (saveLog)
		manager->saveLog();

	delete manager;
}

void onCompressCBClicked()
{
	enableCompressorGroup(SendMessage(compressCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void onSaveLogCBClicked()
{
	saveLog = (SendMessage(saveLogCB, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

bool isFileExists(PCHAR name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}