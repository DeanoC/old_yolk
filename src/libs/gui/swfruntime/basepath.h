// 
//  SwfBasePath.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_BASEPATH_H_
#define YOLK_GUI_SWFRUNTIME_BASEPATH_H_

#include "gui/SwfParser/SwfVec2Twip.h"
#include "core/resources.h"
#include "scene/databuffer.h"

namespace Swf {
	// forward decl
	class FillStyle;

    class QuadraticEdge {
	public:
		QuadraticEdge(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1) {
            straight = true;
            c = _v0 + ((_v1 - _v0) * 0.5f);
            v0 = _v0;
            v1 = _v1;
        }

        QuadraticEdge(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1) {
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
	
	class BasePath {
	public:
		BasePath (FillStyle* _style);
		virtual ~BasePath (){};
		int numIndices;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			vertexBufferHandle;
		Core::ScopedResourceHandle<Scene::DataBufferHandle>			indexBufferHandle;
        int extentsRectVertexOffset;

 		FillStyle* fillStyle;

		virtual void moveTo(const SwfVec2Twip& _v0);

		virtual void lineTo(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1);
		
		virtual void quadraticBezier(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1);
		
		virtual void generateGPUData() = 0;

		bool isSimple() const {
			return simplePolygon;	
		}
	protected:
		bool	simplePolygon;
		std::list<std::vector<QuadraticEdge*>*> polygons;
		std::vector<QuadraticEdge*>* currentPolygon;
	};
} /* Swf */


#endif /* _SWFBASEPATH_H_ */

