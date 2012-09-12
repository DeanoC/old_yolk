// 
//  SwfBasePath.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFBASEPATH_H_
#define _SWFBASEPATH_H_

#include "gui/SwfParser/SwfVec2Twip.h"
namespace Swf
{
	// forward decl
	class SwfRuntimeFillStyle;

    class QuadraticEdge {
	public:
		QuadraticEdge(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1)
        {
            straight = true;
            c = _v0 + ((_v1 - _v0) * 0.5f);
            v0 = _v0;
            v1 = _v1;
        }

        QuadraticEdge(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1)
        {
            straight = false;
            v0 = _v0;
            c = _c;
            v1 = _v1;
        }

        bool straight;
        SwfVec2Twip v0;
        SwfVec2Twip c;
        SwfVec2Twip v1;
        uint16_t i0;
        uint16_t i1;
    };
	
	class SwfBasePath
	{
	public:
		SwfBasePath (SwfRuntimeFillStyle* _style);
		virtual ~SwfBasePath (){};
		int numIndices;
		uint32_t vertexBuffer;
		uint32_t indexBuffer;
        int extentsRectVertexOffset;

 		SwfRuntimeFillStyle* fillStyle;

		virtual void MoveTo(const SwfVec2Twip& _v0);

		virtual void LineTo(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1);
		
		virtual void QuadraticBezier(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1);
		
		virtual void GenerateGPUData() = 0;

		bool IsSimple() const {
			return simplePolygon;	
		}
	protected:
		bool	simplePolygon;
		std::list<std::vector<QuadraticEdge*>*> polygons;
		std::vector<QuadraticEdge*>* currentPolygon;
	};
} /* Swf */


#endif /* _SWFBASEPATH_H_ */

