// 
//  debug_std.h
//  Wierd Core
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2008 Cloud Pixies Ltd. All rights reserved.
//

#pragma once

#ifndef _DEBUG_STD_H_
#define _DEBUG_STD_H_


#include <iostream>



//---------------------------------------------------------------------------
//!	Output to the debug console the type
//---------------------------------------------------------------------------
template<class T>
void DebugInfo(T& in)
{
	std::cerr << in;
}
//---------------------------------------------------------------------------
//!	Output to the debug console the type
//---------------------------------------------------------------------------
template<class T>
void DebugError(T& in)
{
	std::cerr << "Warning: " << in;
}

inline void DebugLogNL()
{
	std::cerr << "\n";
}


#endif /* _DEBUG_STD_H_ */
