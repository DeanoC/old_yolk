// 
//  SwfGpuLinePath.cpp
//  Projects
//  
//  Created by Deano on 2008-10-20.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "SwfRuntimeUtils.h"
#include "core/mathconsts.h"
#include "core/convextest.h"
/*
#if PLATFORM == IPHONE
#include <OpenGLES/ES1/gl.h>
#elif PLATFORM == APPLE_MAC
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
*/
#include "SwfGpuLinePath.h"

namespace Swf
{
	static const float s_tolerance = 1.0f;

	void SwfGpuLinePath::LineTo(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1) {
		pathVertices.push_back(_v0);
		pathVertices.push_back(_v1);
	}
	void SwfGpuLinePath::GenerateGPUData() {
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
			/*
			GLuint names[2];
			CALL_GL(glGenBuffers(2, names));
			CALL_GL(glBindBuffer(GL_ARRAY_BUFFER, names[0]));
			CALL_GL(glBufferData(GL_ARRAY_BUFFER, polysSizeInBytes, tmp, GL_STATIC_DRAW));
			CALL_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, names[1]));
			CALL_GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * destIndexData.size(), &destIndexData[0], GL_STATIC_DRAW));
			CALL_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
			CALL_GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
			vertexBuffer = names[0];
			indexBuffer = names[1];
			numIndices = destIndexData.size();
			*/
			TODO_ASSERT( false && "GL code" );
		}
	}

	// Recursive routine to generate bezier curve within tolerance.
	void SwfGpuLinePath::SubDivCurve(const SwfVec2Twip& _p0, const SwfVec2Twip& _p1, const SwfVec2Twip& _p2) {
		// Midpoint on line between two endpoints.
		SwfVec2Twip mid = Mul( Add(_p0, _p2), 0.5f);

		// Midpoint on the curve.
		SwfVec2Twip q = Mul( Add(mid, _p1), 0.5f);

		float dist = (float)( abs(mid.x - q.x) + abs(mid.y - q.y) );

		if (dist < s_tolerance) {
			// Emit edge.
			LineTo(_p0, _p2);
		} else {
			// Error is too large; subdivide.
			SubDivCurve(_p0, Mul(Add(_p0,_p1),0.5f), q);
			SubDivCurve(q, Mul(Add(_p1,_p2),0.5f), _p2);
		}
	}
	
} /* Swf */ 
