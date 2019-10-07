
#define WIN32_LEAN_AND_MEAN

// Standard packages
#include <windows.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <objidl.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// Engine headers
#include "Resource.h"
#include "v3ctor.h"
#include "engine.h"

// GDI+
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib, "Gdiplus.lib")

// Window Settings
#define MAX_LOADSTRING 100

// Math Constants
#define PI 3.141592635897931
#define ALPHA 0.00001

// Motion constants
#define DELTA_THETA 0.1
#define DELTA_ZOOM 20
#define DELTA_X 75

// Core engine
int* col_bits;
bool running;
bool gin_on;
engine gin;

// Window information
static TCHAR szWindowClass[] = _T("3D Graphics Engine");
static TCHAR szTitle[] = _T("3D Graphics Engine");
HINSTANCE hInst;
HBITMAP hBit;
HWND hWnd;
HDC hDc;
HDC bDc;

// Forward declarations of functions
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID OnPaint(HDC hdc);
void testAxes();


int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{

	WNDCLASSEX wcex;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	//Initialize GDI+
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//Setup
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY3DENGINE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	//Notify in case of failure
	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			szTitle,
			NULL);

		return 1;
	}

	//Store instance handle
	hInst = hInstance;

	//Create the window
	hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	//Notify in case of failure
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			szTitle,
			NULL);
		return 1;
	}

	//Maximize window
	ShowWindow(
		hWnd,
		SW_MAXIMIZE);
	UpdateWindow(hWnd);
	hDc = GetDC(hWnd);
	bDc = CreateCompatibleDC(hDc);

	//Initialize core engine
	RECT r;
	GetWindowRect(hWnd, &r);
	gin = engine(r.right - r.left, r.bottom - r.top, hWnd);
	gin.bmp = new Bitmap(CreateCompatibleBitmap(GetDC(hWnd), gin.winx, gin.winy), NULL);
	gin.pos = v3ctor(-3500, 250, 1000);
	gin.dir = v3ctor(900, 0, 0);
	gin.up = v3ctor(0, 0, 100);
	v3ctor c = cross(gin.dir, gin.up);
	gin.dir = rot(gin.dir, c, -0.2);
	gin.up = rot(gin.up, c, -0.2);
	testAxes();
	running = 1;
	gin_on = 1;

	//Mainloop
	MSG msg;
	bool first = 1;
	while (running)
	{

		//Paint first time
		if (first) {
			OnPaint(hDc);
			first = 0;
		}

		//Handle messages
		if (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Handle Key Input
		bool needs_repaint = false;
		///Move forward
		if (GetAsyncKeyState('W') && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.pos += unit(gin.dir) * DELTA_X;
			needs_repaint = true;
		}
		///Move backward
		if (GetAsyncKeyState('S') && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.pos -= unit(gin.dir) * DELTA_X;
			needs_repaint = true;
		}
		///Move right
		if (GetAsyncKeyState('D')) {
			gin.pos += unit(cross(gin.dir, gin.up)) * DELTA_X;
			needs_repaint = true;
		}
		///Move left
		if (GetAsyncKeyState('A')) {
			gin.pos -= unit(cross(gin.dir, gin.up)) * DELTA_X;
			needs_repaint = true;
		}
		///Move up
		if (GetAsyncKeyState('W') && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.pos += unit(gin.up) * DELTA_X;
			needs_repaint = true;
		}
		///Move down
		if (GetAsyncKeyState('S') && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.pos -= unit(gin.up) * DELTA_X;
			needs_repaint = true;
		}
		///Rotate yaw (right)
		if (GetAsyncKeyState(VK_RIGHT) && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			v3ctor v = gin.pos + gin.dir;
			gin.dir = rot(gin.dir, gin.up, -DELTA_THETA);
			gin.pos = v - gin.dir;
			needs_repaint = true;
		}
		///Rotate yaw (left)
		if (GetAsyncKeyState(VK_LEFT) && !(GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			v3ctor v = gin.pos + gin.dir;
			gin.dir = rot(gin.dir, gin.up, DELTA_THETA);
			gin.pos = v - gin.dir;
			needs_repaint = true;
		}
		///Rotate pitch (up)
		if (GetAsyncKeyState(VK_UP)) {
			v3ctor v = gin.pos + gin.dir;
			gin.dir = rot(gin.dir, cross(gin.dir, gin.up), DELTA_THETA);
			gin.up = rot(gin.up, cross(gin.dir, gin.up), DELTA_THETA);
			gin.pos = v - gin.dir;
			needs_repaint = true;
		}
		///Rotate pitch (down)
		if (GetAsyncKeyState(VK_DOWN)) {
			v3ctor v = gin.pos + gin.dir;
			gin.dir = rot(gin.dir, cross(gin.dir, gin.up), -DELTA_THETA);
			gin.up = rot(gin.up, cross(gin.dir, gin.up), -DELTA_THETA);
			gin.pos = v - gin.dir;
			needs_repaint = true;
		}
		///Rotate roll (right)
		if (GetAsyncKeyState(VK_RIGHT) && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.up = rot(gin.up, gin.dir, DELTA_THETA);
			needs_repaint = true;
		}
		///Rotate roll (left)
		if (GetAsyncKeyState(VK_LEFT) && (GetAsyncKeyState(VK_SHIFT) || GetAsyncKeyState(VK_RSHIFT))) {
			gin.up = rot(gin.up, gin.dir, -DELTA_THETA);
			needs_repaint = true;
		}
		///Zoom in
		if (GetAsyncKeyState(VK_OEM_PLUS)) {
			gin.pos -= unit(gin.dir) * DELTA_ZOOM;
			gin.dir += unit(gin.dir) * DELTA_ZOOM;
			needs_repaint = true;
		}
		///Zoom out
		if (GetAsyncKeyState(VK_OEM_MINUS)) {
			if (abs(gin.dir) > DELTA_ZOOM) {
				gin.pos += unit(gin.dir) * DELTA_ZOOM;
				gin.dir -= unit(gin.dir) * DELTA_ZOOM;
				needs_repaint = true;
			}
		}
		///Repaint
		if (GetAsyncKeyState(VK_SPACE) || needs_repaint)
			OnPaint(hDc);
		///Exit
		if (GetAsyncKeyState(VK_ESCAPE))
			running = false;

	}

	//Shut down GDI+ and exit
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;

}

LRESULT CALLBACK WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{

	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		if (!gin_on) {
			OnPaint(BeginPaint(hWnd, &ps));
			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ERASEBKGND:
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;

}

VOID OnPaint(HDC hdc)
{

	if (gin_on) {

		///Cache z-buffer
		gin.fill_buff();

		///Display buffer
		HBITMAP tBit = hBit;
		gin.bmp->GetHBITMAP(0x00000000, &hBit);
		SelectObject(bDc, hBit);
		BitBlt(
			hdc,
			0, 0,
			gin.winx,
			gin.winy,
			bDc,
			0, 0,
			SRCCOPY
		);
		DeleteObject(tBit);

	}

}

void testAxes()
{

	/*/ big points
	gin.forms.push_back(new big_point(point(500, 500, 500, 0x00FF0000, &gin), &gin));
	gin.forms.push_back(new big_point(point(500, 500, -500, 0x0000FF00, &gin), &gin));
	gin.forms.push_back(new big_point(point(500, -500, 500, 0x000000FF, &gin), &gin));
	gin.forms.push_back(new big_point(point(500, -500, -500, 0x00FFFF00, &gin), &gin));
	gin.forms.push_back(new big_point(point(-500, 500, 500, 0x00FF00FF, &gin), &gin));
	gin.forms.push_back(new big_point(point(-500, 500, -500, 0x0000FFFF, &gin), &gin));
	gin.forms.push_back(new big_point(point(-500, -500, 500, 0x00FF8000, &gin), &gin));
	gin.forms.push_back(new big_point(point(-500, -500, -500, 0x00804000, &gin), &gin));
	//*/

	//*/ sphere points
	gin.forms.push_back(new sphere_point(point(500, 500, 500, 0x00FF0000, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(500, 500, -500, 0x0000FF00, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(500, -500, 500, 0x000000FF, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(500, -500, -500, 0x00FFFF00, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(-500, 500, 500, 0x00FF00FF, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(-500, 500, -500, 0x0000FFFF, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(-500, -500, 500, 0x00FF8000, &gin), 10, &gin));
	gin.forms.push_back(new sphere_point(point(-500, -500, -500, 0x00804000, &gin), 10, &gin));
	//*/

	/*/ tetrahedron
	gin.forms.push_back(new tri(point(100, 0, -100 / sqrt(2), &gin), point(-100, 0, -100 / sqrt(2), &gin), point(0, 100, 100 / sqrt(2), &gin), 0x00FF0000, &gin));
	gin.forms.push_back(new tri(point(100, 0, -100 / sqrt(2), &gin), point(0, -100, 100 / sqrt(2), &gin), point(0, 100, 100 / sqrt(2), &gin), 0x0000FF00, &gin));
	gin.forms.push_back(new tri(point(-100, 0, -100 / sqrt(2), &gin), point(100, 0, -100 / sqrt(2), &gin), point(0, 100, 100 / sqrt(2), &gin), 0x00FF00FF, &gin));
	gin.forms.push_back(new tri(point(100, 0, -100 / sqrt(2), &gin), point(-100, 0, -100 / sqrt(2), &gin), point(0, -100, 100 / sqrt(2), &gin), 0x000000FF, &gin));
	//*/

	// test tri
	//gin.forms.push_back(new tri(point(900, 900, 900, &gin), point(1000, 900, 900, &gin), point(900, 900, 1000, &gin), 0x00FFFFFF, &gin));

	//*/ wireframe cube
	gin.forms.push_back(new line(point(-500, -500, -500, &gin), point(-500, 500, -500, &gin), &gin));
	gin.forms.push_back(new line(point(-500, -500, -500, &gin), point(500, -500, -500, &gin), &gin));
	gin.forms.push_back(new line(point(500, 500, -500, &gin), point(-500, 500, -500, &gin), &gin));
	gin.forms.push_back(new line(point(500, 500, -500, &gin), point(500, -500, -500, &gin), &gin));

	gin.forms.push_back(new line(point(-500, -500, 500, &gin), point(-500, 500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(-500, -500, 500, &gin), point(500, -500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(500, 500, 500, &gin), point(-500, 500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(500, 500, 500, &gin), point(500, -500, 500, &gin), &gin));

	gin.forms.push_back(new line(point(-500, -500, -500, &gin), point(-500, -500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(-500, 500, -500, &gin), point(-500, 500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(500, -500, -500, &gin), point(500, -500, 500, &gin), &gin));
	gin.forms.push_back(new line(point(500, 500, -500, &gin), point(500, 500, 500, &gin), &gin));
	//*/

	/* line axes
	gin.forms.push_back(new line(point(-10000, 0, 0, &gin), point(10000, 0, 0, &gin), 0x00FF00FF, &gin));
	gin.forms.push_back(new line(point(0, -10000, 0, &gin), point(0, 10000, 0, &gin), 0x00FF00FF, &gin));
	gin.forms.push_back(new line(point(0, 0, -10000, &gin), point(0, 0, 10000, &gin), 0x00FF00FF, &gin));
	//*/

	/*point axes
	int space = 500;
	int count = 2000;

	for (int i = -count; i <= count; i++) {
		gin.forms.push_back(new point(i * space, 0, 0, 0x00FF00FF, &gin));
		gin.forms.push_back(new point(0, i * space, 0, 0x00FF00FF, &gin));
		gin.forms.push_back(new point(0, 0, i * space, 0x00FF00FF, &gin));
	}
	//*/

}