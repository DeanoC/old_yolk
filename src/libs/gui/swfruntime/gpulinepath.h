// 
//  SwfGpuLinePath.h
//  Projects
//  
//  Created by Deano on 2008-10-20.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_GPULINEPATH_H_
#define YOLK_GUI_SWFRUNTIME_GPULINEPATH_H_

#include "gui/SwfParser/SwfVec2Twip.h"
#include "core/resources.h"
#include "scene/databuffer.h"
#include "basepath.h"

namespace Swf {
	// forward decl
	class FillStyle;
	
	// ==================================================================
	// = Gpu Line Path is a class for path implementations that convert 		=
	// = Swf Paths into GPU Lines but CPU subidived edge polygons	=
	// ==================================================================
	class GpuLinePath : public BasePath {
	public:
		GpuLinePath( FillStyle* _style ) : BasePath( _style ) {}
		virtual ~GpuLinePath (){};

		virtual void moveTo( const SwfVec2Twip& _v0 ) {}
		virtual void lineTo( const SwfVec2Twip& _v0, const SwfVec2Twip& _v1 );
		virtual void quadraticBezier( const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1 ) {
			subDivCurve(_v0, _c, _v1);
		}
		virtual void generateGPUData();

		int polysSizeInBytes;
	    
	private:
		void subDivCurve(const SwfVec2Twip& _p0, const SwfVec2Twip& _p1, const SwfVec2Twip& _p2);

		typedef std::vector<SwfVec2Twip> TwipList;
		
		TwipList pathVertices;
		std::vector<uint16_t> destIndexData;
	};
	
} /* Swf */ 


#endif /* end of include guard: SWFGPULINEPATH_H_QTCZBRT */

