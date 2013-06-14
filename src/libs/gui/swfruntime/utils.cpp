// 
//  SwfRuntimeUtils.cpp
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/swfparser/SwfMatrix.h"
#include "gui/swfparser/SwfColourTransform.h"
#include "utils.h"

namespace Swf {
	Math::Matrix4x4 Convert(const SwfMatrix* _matrix) {
		return Math::Matrix4x4( _matrix->scaleX, _matrix->rotateSkew0, 	  	0, 0,
        						_matrix->rotateSkew1, _matrix->scaleY, 	  	0, 0,
								0,						0,			   	  	1, 0,
								_matrix->translateX, _matrix->translateY, 	0, 1 );
	}
	Math::Matrix4x4 Convert(const SwfColourTransform* _ct) {
		return Math::Matrix4x4( _ct->mul[0],	_ct->mul[1], 	_ct->mul[2],	_ct->mul[3],
								_ct->add[0],	_ct->add[1], 	_ct->add[2],	_ct->add[3],
								0,						0,		0,				0,
								0,						0,		0,				0 );
	}
	
	std::string ToLowerIfReq( const std::string& _name, bool _isCaseSensitive ) {
		if( _isCaseSensitive == false ) {
			std::string str = _name;
			std::transform(str.begin(), str.end(), str.begin(), tolower);
			return str;
		} else {
			return _name;
		}
	}
} /* Swf */
