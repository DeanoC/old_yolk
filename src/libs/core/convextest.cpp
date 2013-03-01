// 
//  convextest.cpp
//  deano
//  
//  Created by Deano on 2008-10-12.
//  Copyright 2008 Cloud Pixies Ltd. All rights reserved.
// 

#include "core/core.h"
#include "convextest.h"
#include <stdio.h>
/*
 * C code from the article
 * "Testing the Convexity of a Polygon"
 * by Peter Schorn and Frederick Fisher,
 *	(schorn@inf.ethz.ch, fred@kpc.com)
 * in "Graphics Gems IV", Academic Press, 1994
 */

/* Program to Classify a Polygon's Shape */

typedef struct { float x, y; } Point2d;

struct PointTracker {
	union Data {
		struct {
			float a;
			float b;
		} f;
		Point2d pnt;	
	} *data;
	int nVertices;
	int offset;	
};

/* Given a directed line pq, determine	*/
/* whether qr turns CW or CCW.		*/
static int WhichSide(const Point2d& p, const Point2d& q, const Point2d& r)
{
    float result;
    result = (p.x - q.x) * (q.y - r.y) - (p.y - q.y) * (q.x - r.x);
    if (result < 0) return -1;	/* q lies to the left  (qr turns CW).	*/
    if (result > 0) return  1;	/* q lies to the right (qr turns CCW).	*/
    return 0;			/* q lies on the line from p to r.	*/
}

/* Lexicographic comparison of p and q	*/
static int Compare(const Point2d& p, const Point2d& q)		
{
    if (p.x < q.x) return -1;	/* p is less than q.			*/
    if (p.x > q.x) return  1;	/* p is greater than q.			*/
    if (p.y < q.y) return -1;	/* p is less than q.			*/
    if (p.y > q.y) return  1;	/* p is greater than q.			*/
    return 0;			/* p is equal to q.			*/
}
/* Read p's x- and y-coordinate from f	*/
/* and return true, iff successful.	*/
static bool GetPoint(PointTracker* tracker, Point2d* p)		
{
	if( tracker->offset >= tracker->nVertices){
		return false;
	}
	*p = tracker->data[tracker->offset++].pnt;
	return true;
}
/* Read next point into 'next' until it */
/* is different from 'previous' and	*/
/* return true iff successful.		*/
static bool GetDifferentPoint(PointTracker* tracker, const Point2d& previous, Point2d* next)
{
    while(GetPoint(tracker, next)){
		if(Compare(previous, *next) != 0)
			return true;
	};
	
	return false;
}

/* CheckTriple tests three consecutive points for change of direction
 * and for orientation.
 */
#define CheckTriple							\
	if ( (thisDir = Compare(second, third)) == -curDir )		\
	    ++dirChanges;						\
	curDir = thisDir;						\
	if ( thisSign = WhichSide(first, second, third) ) {		\
	    if ( angleSign == -thisSign )				\
		return PolygonClass::NOT_CONVEX;					\
	    angleSign = thisSign;					\
	}								\
	first = second; second = third;
namespace PolygonClass {

/* Classify the polygon vertices on file 'f' according to: 'NOT_CONVEX'	*/
/* 'NOT_CONVEX_DEGENERATE', 'CONVEX_DEGENERATE', 'CONVEX_CCW', 'CONVEX_CW'.	*/
PolygonClass::Enum ClassifyPolygon( float* Input, int nVertices ) {	
	PointTracker tracker;
	tracker.data = (PointTracker::Data*)Input;
	tracker.nVertices = nVertices;
	tracker.offset = 0;
    int		 curDir, thisDir, thisSign, angleSign = 0, dirChanges = 0;
    Point2d	 first, second, third, saveFirst, saveSecond;

    if ( !GetPoint(&tracker, &first) || !GetDifferentPoint(&tracker, first, &second) )
		return PolygonClass::CONVEX_DEGENERATE;
    saveFirst = first;	saveSecond = second;
    curDir = Compare(first, second);
    while( GetDifferentPoint(&tracker, second, &third) ) {
		CheckTriple;
    }
    /* Must check that end of list continues back to start properly */
    if ( Compare(second, saveFirst) ) {
		third = saveFirst; CheckTriple;
    }
    third = saveSecond;	 CheckTriple;

    if ( dirChanges > 2 ) return angleSign ? PolygonClass::NOT_CONVEX : PolygonClass::NOT_CONVEX_DEGENERATE;
    if ( angleSign  > 0 ) return PolygonClass::CONVEX_CCW;
    if ( angleSign  < 0 ) return PolygonClass::CONVEX_CW;
    return PolygonClass::CONVEX_DEGENERATE;
}

}