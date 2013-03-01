// 
//  SwfRuntimeUtils.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_UTILS_H_
#define YOLK_GUI_SWFRUNTIME_UTILS_H_

#include "gui/SwfParser/SwfVec2Twip.h"

namespace Swf {
	// forward decl
	class SwfMatrix;
	class SwfColourTransform;

	Math::Matrix4x4 Convert(const SwfMatrix* _matrix);

	// NOTE muls are in the row(0), adds are in row(1), last two rows = 0
	Math::Matrix4x4 Convert(const SwfColourTransform* _ct);

	inline bool Equal(const SwfVec2Twip& _a, const SwfVec2Twip& _b) {
		return ((_a.x == _b.x) && (_a.y == _b.y));
	}
	inline bool NotEqual(const SwfVec2Twip& _a, const SwfVec2Twip& _b){
		return ((_a.x != _b.x) || (_a.y != _b.y));		
	}
	
	inline SwfVec2Twip Add(const SwfVec2Twip& _a, const SwfVec2Twip& _b){
		return SwfVec2Twip(_a.x + _b.x, _a.y + _b.y);
	}
	inline SwfVec2Twip Sub(const SwfVec2Twip& _a, const SwfVec2Twip& _b){
		return SwfVec2Twip(_a.x - _b.x, _a.y - _b.y);
	}
	inline SwfVec2Twip Mul(const SwfVec2Twip& _a, const float _s){
		return SwfVec2Twip((int)((float)_a.x * _s), (int)((float)_a.y * _s) );
	}
	inline int Magnitude(const SwfVec2Twip& _a, const SwfVec2Twip& _b){
		return ((_a.x * _b.x) + (_a.y * _b.y));
	}
//	void CheckGL(const char* _call);
//	void TestImage( int textNum, float xScale, float yScale );
	
	std::string ToLowerIfReq( const std::string& _name, bool _isCaseSensitive );
	
} /* Swf */

//#define CALL_GL(call) call; Swf::CheckGL(#call);

#endif /* YOLK_GUI_SWFRUNTIME_UTILS_H_ */
