// 
//  SwfVec2Twip.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_VEC2TWIP_H)
#define _SWF_VEC2TWIP_H

namespace Swf{

	struct SwfVec2Twip
	{
	public:
		int x;
		int y;

		SwfVec2Twip(){}

		SwfVec2Twip(int _x, int _y)
		{
			x = _x;
			y = _y;
		}

	};
	
	static SwfVec2Twip operator +(const SwfVec2Twip& _a, const SwfVec2Twip& _b)
	{
		return SwfVec2Twip(_a.x + _b.x, _a.y + _b.y);
	}
	static SwfVec2Twip operator -(const SwfVec2Twip& _a, const SwfVec2Twip& _b)
	{
		return SwfVec2Twip(_a.x - _b.x, _a.y - _b.y);
	}
	static SwfVec2Twip operator *(const SwfVec2Twip& _a, float _b)
	{
		return SwfVec2Twip( (int)((float)_a.x * _b), (int)((float)_a.y * _b));
	}
	static bool operator == (const SwfVec2Twip& _a, const SwfVec2Twip& _b)
	{
		return ((_a.x == _b.x) && (_a.y == _b.y));
	}
	static bool operator !=(const SwfVec2Twip& _a, const SwfVec2Twip& _b)
	{
		return ((_a.x != _b.x) || (_a.y != _b.y));
	}

}
#endif