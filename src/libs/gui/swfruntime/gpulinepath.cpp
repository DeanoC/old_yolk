// 
//  SwfGpuLinePath.cpp
//  Projects
//  
//  Created by Deano on 2008-10-20.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include <boost/lexical_cast.hpp>
#include "core/mathconsts.h"
#include "core/convextest.h"

#include "utils.h"
#include "gpulinepath.h"

namespace Swf {
	static const float s_tolerance = 1.0f;

	void GpuLinePath::lineTo( const SwfVec2Twip& _v0, const SwfVec2Twip& _v1 ) {
		pathVertices.push_back(_v0);
		pathVertices.push_back(_v1);
	}
	void GpuLinePath::generateGPUData() {
		// 2 floats per vertex + the center fan vertex + 4 for the extents rect
		float* tmp = CORE_NEW_ARRAY float[pathVertices.size()* 2];
		int index = 0;
		for( 	TwipList::const_iterator i = pathVertices.begin(); 
				i != pathVertices.end();
				++i) {
			const SwfVec2Twip& twip = *i;
			tmp[index++] = (float)twip.x;
			tmp[index++] = (float)twip.y;
		}
		Math::Vector2 p1,t;
		// add middle point (we will fan from it)
		for (size_t i = 0; i < pathVertices.size(); i += 2) {
			bool jump = true;
			if(i > 0) {
				p1 = Math::Vector2(tmp[((i-1)*2)+0], tmp[((i-1)*2)+1]);
				t =  Math::Vector2(tmp[((i)*2)+0], tmp[((i)*2)+1]);
				if( p1 == t){
					jump = false;
				}
			}
			if(jump == true) {
				destIndexData.push_back((uint16_t)i);
			} else {
				destIndexData.push_back((uint16_t)(i-1));
			}

			destIndexData.push_back((uint16_t)i+1);
		}

		polysSizeInBytes = sizeof(float) * 2 * pathVertices.size();

		if (destIndexData.size() >= 2) {

			namespace s = Scene;
			s::DataBuffer::CreationInfo vbcs ( s::Resource::BufferCtor(
				s::RCF_BUF_VERTEX | s::RCF_ACE_IMMUTABLE, 
				polysSizeInBytes, tmp 
			) );
			vertexBufferHandle = s::DataBufferHandle::create( "swfgpulinepath_vb", &vbcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );
			s::DataBuffer::CreationInfo ibcs ( s::Resource::BufferCtor(
				s::RCF_BUF_INDEX | s::RCF_ACE_IMMUTABLE, 
				sizeof(uint16_t) * destIndexData.size(), &destIndexData[0]
			) );
			indexBufferHandle = s::DataBufferHandle::create( "swfgpulinepath_ib", &ibcs, Core::RESOURCE_FLAGS::RMRF_DONTCACHE );

			numIndices = destIndexData.size();

		}
	}

	// Recursive routine to generate bezier curve within tolerance.
	void GpuLinePath::subDivCurve(const SwfVec2Twip& _p0, const SwfVec2Twip& _p1, const SwfVec2Twip& _p2) {
		// Midpoint on line between two endpoints.
		SwfVec2Twip mid = Mul( Add(_p0, _p2), 0.5f);

		// Midpoint on the curve.
		SwfVec2Twip q = Mul( Add(mid, _p1), 0.5f);

		float dist = (float)( abs(mid.x - q.x) + abs(mid.y - q.y) );

		if (dist < s_tolerance) {
			// Emit edge.
			lineTo(_p0, _p2);
		} else {
			// Error is too large; subdivide.
			subDivCurve(_p0, Mul(Add(_p0,_p1),0.5f), q);
			subDivCurve(q, Mul(Add(_p1,_p2),0.5f), _p2);
		}
	}
	
} /* Swf */ 
