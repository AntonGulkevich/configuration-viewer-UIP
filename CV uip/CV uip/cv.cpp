#include <windows.h>
#include <string.h>
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



static TCHAR szWindowClass[] = _T("win32app");
static TCHAR szTitle[] = _T("CM loader");

HINSTANCE hInst;
HWND convertCB;
HWND compressCB;
HWND openCM;
HWND openZip;
HWND confWayLE;
HWND zipWayLE;
HWND buttonLoadCM;
HWND dropListDevice;

HBITMAP hBitmap = nullptr;


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void GetFileName(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);


/*events*/

void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);
void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle);

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
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

		return 1;
	}

	hInst = hInstance;
	auto hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT,
		560, 160,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hWnd)
	{
		MessageBox(nullptr,
			_T("Call to CreateWindow failed!"),
			szTitle,
			NULL);

		return 1;
	}

	convertCB = CreateWindow(_T("button"), _T("Сжатие"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		17, 25, 130, 16, hWnd, reinterpret_cast<HMENU>(1), hInst, nullptr);

	compressCB = CreateWindow(_T("button"), _T("Создать архив"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		17, 59, 130, 16, hWnd, reinterpret_cast<HMENU>(2), hInst, nullptr);

	confWayLE = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("D:\\commod.bin\\"),
		WS_VISIBLE | WS_CHILD | ES_LEFT,
		150, 22, 350, 25, hWnd, reinterpret_cast<HMENU>(3), hInst, nullptr);

	zipWayLE = CreateWindowEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T("D:\\Program Files (x86)\\7-Zip\\7z.exe"),
		WS_VISIBLE | WS_CHILD | ES_LEFT,
		150, 55, 350, 25, hWnd, reinterpret_cast<HMENU>(4), hInst, nullptr);

	buttonLoadCM = CreateWindow(_T("button"), _T("Загрузить"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		425, 90, 100, 25, hWnd, reinterpret_cast<HMENU>(5), hInst, NULL);

	openCM = CreateWindow(_T("button"), _T("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		500, 22, 23, 25, hWnd, reinterpret_cast<HMENU>(6), hInst, NULL);

	openZip =  CreateWindow(_T("button"), _T("..."), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		500, 55, 23, 25, hWnd, reinterpret_cast<HMENU>(7), hInst, NULL);

	dropListDevice = CreateWindow(_T("COMBOBOX"),NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWN,
		60, 62, 136, 60, hWnd,NULL,	hInst,	NULL);


	hBitmap = static_cast<HBITMAP>(LoadImage(hInst, _T("D:\\GIT\\configuration-viewer-UIP\\CV uip\\CV uip\\logo-top.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Main message loop:
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
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 6:			
			onConfFileSelected(hWnd, confWayLE,  _T("*.bin"), _T("Выберете файл конфигурации"));
			break;
		case 7:
			onZipFileselected(hWnd, zipWayLE, _T("7z.exe"), _T("Выберете архиватор"));
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
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

void onConfFileSelected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}

void onZipFileselected(HWND hWnd, HWND lineEdit, PTCHAR lpstrFilter, PTCHAR lpstrTitle)
{
	GetFileName(hWnd, lineEdit, lpstrFilter, lpstrTitle);
}