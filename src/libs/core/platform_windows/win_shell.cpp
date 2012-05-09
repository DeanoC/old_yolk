// 
//  win_shell.cpp
//  deano
//  
//  Created by Deano on 2010.
//  Copyright 2010 Zombie House Studios. All rights reserved.
// 

#include "core/core.h"
#include "core/platform_windows/mouse_win.h"
#include "core/platform_windows/keyboard_win.h"
#include "core/sysmsg.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
char		g_moduleFilename[_MAX_PATH];  
HINSTANCE	g_hInstance;
HINSTANCE	g_hPrevInstance;
HWND        g_hWnd = NULL;
int			g_nCmdShow;
extern int Main();


// maximum mumber of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole() {
	using namespace std;
	int hConHandle;
	long lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	// redirect unbuffered STDOUT to the console
	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console
	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console
	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog

	// point to console as well
	ios::sync_with_stdio();

}


//!-----------------------------------------------------
//! Pumps windows messages, also pauses here when we have
//! lost focus
//!-----------------------------------------------------
void WinMainCmdLineToMainArgs(char* command_line) {
	char*  arg;
	int    index;

	// early out if already set up (either double call or via a main -> winmain
	// thikn from a command line app
	if( Core::g_argc != 0 ) {
		return;
	}

	// count the arguments
	Core::g_argc = 1;
	arg  = command_line;
	
	while (arg[0] != 0) {
		while (arg[0] != 0 && arg[0] == ' ') {
			arg++;
		}

		if (arg[0] != 0) {      
			Core::g_argc++;        
			while (arg[0] != 0 && arg[0] != ' ') {
				arg++;
			}
		}
	}    

	if( Core::g_argc > Core::MAX_CMDLINE_ARGS ) {
		Core::g_argc = Core::MAX_CMDLINE_ARGS;
	}

	// tokenize the arguments
	arg = command_line;
	index = 1;

	while (arg[0] != 0) {
		while (arg[0] != 0 && arg[0] == ' ') {
			arg++;
		}

		if (arg[0] != 0) {
			Core::g_argv[index] = arg;
			index++;
		
			while (arg[0] != 0 && arg[0] != ' ') {
				arg++;
			}
		
			if (arg[0] != 0) {
				arg[0] = 0;    
				arg++;
			}
		}
	}    

	// put the program name into argv[0]
	GetModuleFileNameA(NULL, g_moduleFilename, _MAX_PATH);
	Core::g_argv[0] = g_moduleFilename;
}


//!-----------------------------------------------------
//! Pumps windows messages, also pauses here when we have
//! lost focus
//!-----------------------------------------------------
void WinGetMessages( void ) {
	if( g_hWnd == NULL ) {
		return;
	}

	MSG Message;
	while (PeekMessage(&Message, NULL, 0, 0,PM_REMOVE) )
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if( Core::MouseWin::exists() ) {
		Core::MouseWin::get()->processMouseMessages( message, wParam, lParam );
	}
	if( Core::KeyboardWin::exists() ) {
		Core::KeyboardWin::get()->processKeyMessages( message, wParam, lParam );
	}

	switch (message) 
	{
	case WM_DESTROY:
		Core::SystemMessage::get()->Quit();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
bool WinInitWindow( int width, int height, bool bFullscreen ) {
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = g_hInstance;
	wcex.hIcon = NULL;//LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "Wierd";
	wcex.hIconSm = NULL;//LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
	if( !RegisterClassEx( &wcex ) )
		return false;

	// Create window
	RECT rc = { 0, 0, width, height };
	if( bFullscreen == false ) {
		AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	}
	g_hWnd = CreateWindow( "Wierd", "Wierd", 
							WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, g_hInstance,
						   NULL );
	if( !g_hWnd )
		return false;

	ShowWindow( g_hWnd, g_nCmdShow );

	return true;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop.
//--------------------------------------------------------------------------------------
int WINAPI WinMain(         HINSTANCE hInstance,
							HINSTANCE hPrevInstance,
							LPSTR lpCmdLine,
							int nCmdShow ) {

	WinMainCmdLineToMainArgs( lpCmdLine );
#if defined( USE_GLOG )
	google::InitGoogleLogging(Core::g_argv[0]);
#endif

	Core::Init();
	IF_DEBUG(
		RedirectIOToConsole();
	);

	g_hInstance = hInstance;
	g_hPrevInstance = hPrevInstance;
	g_nCmdShow = nCmdShow;
	
	int res = Main();

	Core::Shutdown();

	return res;
}