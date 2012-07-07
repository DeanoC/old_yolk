///-------------------------------------------------------------------------------------------------
/// \file	platform_windows\win_shell.h
///
/// \brief	Declares the window shell class. 
///
/// \details	
///		win_shell description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description. 
////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef WEIRD_CORE_WIN_SHELL_H
#define WEIRD_CORE_WIN_SHELL_H

extern bool WinInitWindow( int width, int height, bool bFullscreen );

extern HWND g_hWnd;

#endif