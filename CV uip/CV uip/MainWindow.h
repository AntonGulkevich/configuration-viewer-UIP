#ifndef MAINWINDOW_UIP_H
#define MAINWINDOW_UIP_H

#include <windows.h>
#include <tchar.h>


class MainWindow
{
public:
	MainWindow(char* caption, HINSTANCE hInstance, int nCmdShow, LPCSTR Menu);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	~MainWindow();
private:
	WNDCLASSEX wcex;
	HINSTANCE hInst;

};

#endif

