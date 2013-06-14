// 
//  SwfGpuPath.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_GPUPATH_H_
#define YOLK_GUI_SWFRUNTIME_GPUPATH_H_

#include "gui/swfparser/SwfVec2Twip.h"
#include "basepath.h"

namespace Swf {
	// forward decl
	class FillStyle;
	
	// ==================================================================
	// = Gpu Path is a class for path implementations that convert 		=
	// = Swf Paths into GPU tesselated but CPU subidived edge polygons	=
	// ==================================================================
	class GpuPath : public BasePath {
	public:
		GpuPath( FillStyle* _style ) : BasePath(_style) {}
		virtual ~GpuPath (){};

		int polysSizeInBytes;
		virtual void moveTo( const SwfVec2Twip& _v0) {}
		virtual void lineTo( const SwfVec2Twip& _v0, const SwfVec2Twip& _v1);
		virtual void quadraticBezier( const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1 ) {
			subDivCurve(_v0, _c, _v1);
		}
		virtual void generateGPUData(); 
	private:
		void subDivCurve( const SwfVec2Twip& _p0, const SwfVec2Twip& _p1, const SwfVec2Twip& _p2 );

		typedef std::vector<SwfVec2Twip> TwipList;
		
		TwipList pathVertices;
		std::vector<uint16_t> destIndexData;

	};
} /* Swf */


#endif /* _SWFGPUPATH_H_ */
