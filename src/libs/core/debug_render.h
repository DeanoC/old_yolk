//!-----------------------------------------------------
//!
//! \file debug_render.h
//! this is a thunk class, to allow any system to render
//! stuff for debug purposes without having to know about
//! the renderer.
//! The actual renderer installs a interface class that
//! does the work with no module dependecies.
//! For obvious reason this is not nesscarily fast and
//! calss will be ignored in real release mode
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_DEBUG_RENDER_H
#define WIERD_CORE_DEBUG_RENDER_H


#include "vector_math.h"
#include "colour.h"
#include <cstdarg>


namespace Core {


class DebugRenderInterface
{
public:
	//! draw a NDC (projection) space line (resolution independent)
	virtual void ndcLine( const Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) = 0;
	//! draw a 3D world space line
	virtual void worldLine( const Colour& colour, const Math::Vector3& a, const Math::Vector3& b ) = 0;
	//! print some text onto some form of screen console
	virtual void print( const char* pText ) = 0;

	void printf( const char *pStr, ... ) {
		char pBuffer[1024];
		va_list marker;
		va_start( marker, pStr );
		vsprintf( pBuffer, pStr, marker );
	
		print( pBuffer );
	
		va_end( marker );
	}

	// supports a small number of prints (stay on screen until changed)
	virtual int getNumberOfVarPrints() const = 0;
	virtual void varPrint( const int _index, const Math::Vector2& _pos, const char* _text ) = 0;
	virtual void varPrintf( const int _index, const Math::Vector2& _pos, const char* _text, ... ) {
		char pBuffer[1024];
		va_list marker;
		va_start( marker, _text );
		vsprintf( pBuffer, _text, marker );
	
		varPrint( _index, _pos, pBuffer );
	
		va_end( marker );
	}

	//! world sphere
	virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& localPos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix()  ) = 0;
	//! world cylinder, cylinders are along the z
	virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) = 0;
	//! world box
	virtual void worldBox( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) = 0;

	void worldBox( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const Math::Vector3& len, const Math::Matrix4x4& transform = Math::IdentityMatrix() ){
		worldBox( colour, localPos, localOrient, len.x, len.y, len.z, transform );
	}

	void worldRect( const Colour& colour, const Math::Vector3& a, const Math::Vector3& b, const Math::Vector3& c, const Math::Vector3& d ) {
		worldLine( colour, a , b );
		worldLine( colour, b , c );
		worldLine( colour, c , d );
		worldLine( colour, d , a );
	}

};


// this is actual renderer, the ctor of the actual debug renderer singleton should make it doing somethin
extern DebugRenderInterface* g_pDebugRender;


}	//namespace Core


#endif