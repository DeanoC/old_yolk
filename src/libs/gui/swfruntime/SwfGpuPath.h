// 
//  SwfGpuPath.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFGPUPATH_H_
#define _SWFGPUPATH_H_

#include "gui/SwfParser/SwfVec2Twip.h"
#include "SwfBasePath.h"
namespace Swf {
	// forward decl
	class SwfRuntimeFillStyle;
	
	// ==================================================================
	// = Gpu Path is a class for path implementations that convert 		=
	// = Swf Paths into GPU tesselated but CPU subidived edge polygons	=
	// ==================================================================
	class SwfGpuPath : public SwfBasePath {
	public:
		SwfGpuPath(SwfRuntimeFillStyle* _style) :
			SwfBasePath(_style) {
		}
		virtual ~SwfGpuPath (){};

		int polysSizeInBytes;
		virtual void MoveTo(const SwfVec2Twip& _v0) {}
		virtual void LineTo(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1);
		virtual void QuadraticBezier(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1) {
			SubDivCurve(_v0, _c, _v1);
		}
		virtual void GenerateGPUData(); 
	private:
		void SubDivCurve(const SwfVec2Twip& _p0, const SwfVec2Twip& _p1, const SwfVec2Twip& _p2);

		typedef std::vector<SwfVec2Twip> TwipList;
		
		TwipList pathVertices;
		std::vector<uint16_t> destIndexData;
	};
} /* Swf */


#endif /* _SWFGPUPATH_H_ */
