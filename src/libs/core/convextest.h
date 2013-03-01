///-------------------------------------------------------------------------------------------------
/// \file	core\convextest.h
///
/// \brief	Declares the convextest class. 
///
/// \details	
///		convextest description goes here
///
/// \remark	Copyright (c) 2012 Cloud Pixies Ltd. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
/// \todo	Fill in detailed file description. 
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CONVEXTEST_H
#define CONVEXTEST_H

///-------------------------------------------------------------------------------------------------
/// \namespace	PolygonClass
///
/// \brief	.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace PolygonClass {

///-------------------------------------------------------------------------------------------------
/// \enum	Enum
///
/// \brief	Values that represent Enum. 
///
/// \details	convextest description goes here. 
/// \todo	Fill in detailed enum descriptions. 
////////////////////////////////////////////////////////////////////////////////////////////////////
enum Enum {
	NOT_CONVEX,
	NOT_CONVEX_DEGENERATE,
    CONVEX_DEGENERATE,
	CONVEX_CCW,
	CONVEX_CW
};


PolygonClass::Enum ClassifyPolygon( float* Input, int nVertices );

}	//namespace PolygonClass


#endif
