// 
//  AsByteCode.h
//  «project»
//  
//  Created by Deano on 2008-11-16.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#ifndef ASBYTECODE_H_1TOOCEII
#define ASBYTECODE_H_1TOOCEII


namespace Swf
{
	enum AsByteCode
    {
#define DECLARE_BYTECODE( name, val ) name = val,
		#include "AsByteCode_inc.h"
#undef DECLARE_BYTECODE
	};
} /* Swf */ 



#endif /* end of include guard: ASBYTECODE_H_1TOOCEII */
