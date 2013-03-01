/*
 *  SwfBasePath.cpp
 *
 *  Created by Deano on 28/09/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "utils.h"
#include "basepath.h"
namespace Swf {
	BasePath::BasePath( FillStyle* _style ) {
		fillStyle = _style;
		simplePolygon = false;
	}
	void BasePath::moveTo( const SwfVec2Twip& _v0 ) {
	    currentPolygon = CORE_NEW std::vector<QuadraticEdge*>();
	    polygons.push_back(currentPolygon);
	}
	void BasePath::lineTo( const SwfVec2Twip& _v0, const SwfVec2Twip& _v1 ) {
	    if (currentPolygon->size() > 0) {
	        if ( NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v1) &&
	             NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v0) ) {
	            moveTo(_v0);
	        }
	    }
	    currentPolygon->push_back(CORE_NEW QuadraticEdge(_v0, _v1));
	}

	void BasePath::quadraticBezier( const SwfVec2Twip& _v0, const SwfVec2Twip& _c, const SwfVec2Twip& _v1 ) {
	    if (currentPolygon->size() > 0) {
	        if ( NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v1) &&
	             NotEqual(_v0, (*currentPolygon)[currentPolygon->size() - 1]->v0) ) {
	            moveTo(_v0);
	        }
	    }
	    currentPolygon->push_back(CORE_NEW QuadraticEdge(_v0, _c, _v1));
	}
} /* Swf */
