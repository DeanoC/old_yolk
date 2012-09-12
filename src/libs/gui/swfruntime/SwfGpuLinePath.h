// 
//  SwfGpuLinePath.h
//  Projects
//  
//  Created by Deano on 2008-10-20.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef SWFGPULINEPATH_H_QTCZBRT
#define SWFGPULINEPATH_H_QTCZBRT

#include "gui/SwfParser/SwfVec2Twip.h"
#include "SwfBasePath.h"

namespace Swf {
	// forward decl
	class SwfRuntimeFillStyle;
	
	// ==================================================================
	// = Gpu Line Path is a class for path implementations that convert 		=
	// = Swf Paths into GPU Lines but CPU subidived edge polygons	=
	// ==================================================================
	class SwfGpuLinePath : public SwfBasePath {
	public:
		SwfGpuLinePath(SwfRuntimeFillStyle* _style) :
		SwfBasePath(_style) {
		}
		virtual ~SwfGpuLinePath (){};

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


#endif /* end of include guard: SWFGPULINEPATH_H_QTCZBRT */

