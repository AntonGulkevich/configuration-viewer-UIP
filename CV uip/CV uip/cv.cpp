#include <windows.h>
#include <string.h>
#include <commctrl.h>
#include <tchar.h>

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

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

/*operations*/
void GetFileName(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void initControls(HWND hWnd);
HWND initMainWindow(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine);
/*end of operations*/

/*events*/
void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
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
		case 6: 
			SendMessage(GetDlgItem(hWnd, 14), PBS_SMOOTH, 0, 0);
			SendMessage(GetDlgItem(hWnd, 14), PBM_SETSTEP, 100, 0);
			SendMessage(GetDlgItem(hWnd, 14), PBM_STEPIT, 0, 0);
			break;
		case 7:			
			onConfFileSelected(hWnd, confWayLE,  _T("*.bin"), _T("Выбор файла конфигурации"));
			break;
		case 8:
			onZipFileselected(hWnd, zipWayLE, _T("7z.exe"), _T("Выбор архиватора"));
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
	TCHAR buf[1000] = { 0 };
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = hInst;
	ofn.lpstrFilter = lpstrFilter; 
	ofn.lpstrTitle = lpstrTitle; 
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	GetOpenFileName(&ofn) ? _tcscpy_s(buf, szFileName) : GetWindowText(lineEdit, buf, 1000);
	SetWindowText(lineEdit, buf);
	EnableWindow(hWnd, true);
	SetFocus(hWnd);
}

void initControls(HWND hWnd)
{	
	/*check boxes*/
	convertCB = CreateWindow(_T("button"), _T("Сжатие"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		10, 90, 100, 16, hWnd, reinterpret_cast<HMENU>(1), hInst, nullptr);
	compressCB = CreateWindow(_T("button"), _T("Создать архив"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		120, 90, 130, 16, hWnd, reinterpret_cast<HMENU>(2), hInst, nullptr);
	saveLogCB = CreateWindow(_T("button"), _T("Сохранить лог"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		270, 90, 130, 16, hWnd, reinterpret_cast<HMENU>(3), hInst, nullptr);
	/*end of check boxes*/

	/*static labels*/
	configSL = CreateWindow( _T("STATIC"), _T("Конфигурация:"), WS_CHILD | WS_VISIBLE,
		10, 23, 100, 20, hWnd, reinterpret_cast<HMENU>(11), hInst, NULL);
	compressorSL = CreateWindow(_T("STATIC"), _T("Архиватор:"), WS_CHILD | WS_VISIBLE,
		10, 56, 100, 20, hWnd, reinterpret_cast<HMENU>(12), hInst, NULL);
	deviceSL = CreateWindow(_T("STATIC"), _T("Устройство:"), WS_CHILD | WS_VISIBLE,
		10, 123, 100, 20, hWnd, reinterpret_cast<HMENU>(13), hInst, NULL);
	/*end of static lables*/

	/*line edits*/
	confWayLE = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("D:\\commod.bin\\"),
		WS_VISIBLE | WS_CHILD | ES_LEFT,
		120, 20, 410, 25, hWnd, reinterpret_cast<HMENU>(4), hInst, nullptr);
	zipWayLE = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("D:\\Program Files (x86)\\7-Zip\\7z.exe"),
		WS_VISIBLE | WS_CHILD | ES_LEFT,
		120, 55, 410, 25, hWnd, reinterpret_cast<HMENU>(5), hInst, nullptr);
	/*end of line edits*/

	/*push buttons*/
	loadCMPB = CreateWindow(_T("button"), _T("Загрузить"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		450, 155, 100, 25, hWnd, reinterpret_cast<HMENU>(6), hInst, NULL);
	openCMPB = CreateWindow(_T("button"), _T("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		530, 20, 23, 25, hWnd, reinterpret_cast<HMENU>(7), hInst, NULL);
	openZipPB = CreateWindow(_T("button"), _T("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		530, 55, 23, 25, hWnd, reinterpret_cast<HMENU>(8), hInst, NULL);
	refreshDevicesPB = CreateWindow(_T("button"), _T("Обновить"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		450, 119, 100, 25, hWnd, reinterpret_cast<HMENU>(9), hInst, NULL);
	/*end of push buttons*/

	/*drop lists*/
	devicesDL = CreateWindow(_T("COMBOBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
		120, 120, 320, 200, hWnd, reinterpret_cast<HMENU>(10), hInst, NULL);
	/*end of drop lists*/

	/*images and icons*/
	hBitmap = static_cast<HBITMAP>(LoadImage(hInst, _T("D:\\GIT\\configuration-viewer-UIP\\CV uip\\CV uip\\logo-top.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	/*end of images and icons*/

	/*progress bar*/
	hProgBar = CreateWindowEx(0, PROGRESS_CLASS, nullptr, WS_CHILD | WS_VISIBLE,
		10, 157, 430, 20, hWnd, reinterpret_cast<HMENU>(14), hInst, nullptr);
	/*end of progress bar*/
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
		MessageBox(nullptr,
			_T("Call to RegisterClassEx failed!"),
			szTitle,
			NULL);

		return nullptr;
	}

	hInst = hInstance;
	auto hWnd = CreateWindow(szWindowClass, szTitle,
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT,
		590, 230, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		MessageBox(nullptr,
			_T("Call to CreateWindow failed!"),
			szTitle,
			NULL);

		return nullptr;
	}
	return hWnd;
}

void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}

void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}