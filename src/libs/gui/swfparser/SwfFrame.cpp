// 
//  SwfFrame.cpp
//  SwfParser
//  
//  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"
#include "swfcontrolelement.h"
#include "SwfFrame.h"

namespace Swf {
	SwfFrame::~SwfFrame () {
		for( auto i : frameElements ) {
			CORE_DELETE( i );
		}
	}

}