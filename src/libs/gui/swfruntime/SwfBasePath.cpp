/*
 *  SwfBasePath.cpp
 *  SkyGlow-Iphone
 *
 *  Created by Deano on 28/09/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "SwfBasePath.h"
#include "SwfRuntimeUtils.h"
namespace Swf
{
	SwfBasePath::SwfBasePath(SwfRuntimeFillStyle* _style) {
		fillStyle = _style;
		simplePolygon = false;
	}
	void SwfBasePath::MoveTo(const SwfVec2Twip& _v0) {
	    currentPolygon = CORE_NEW std::vector<QuadraticEdge*>();
	    polygons.push_back(currentPolygon);
	}
	void SwfBasePath::LineTo(const SwfVec2Twip& _v0, const SwfVec2Twip& _v1) {
	    if (currentPolygon->size() > 0) {
	        if ( NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v1) &&
	             NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v0) ) {
	            MoveTo(_v0);
	        }
	    }
	    currentPolygon->push_back(CORE_NEW QuadraticEdge(_v0, _v1));
	}

	void SwfBasePath::QuadraticBezier(const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1) {
	    if (currentPolygon->size() > 0) {
	        if ( NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v1) &&
	             NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v0) ) {
	            MoveTo(_v0);
	        }
	    }
	    currentPolygon->push_back(CORE_NEW QuadraticEdge(_v0, _c, _v1));
	}
} /* Swf */
