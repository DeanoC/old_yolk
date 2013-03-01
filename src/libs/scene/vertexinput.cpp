/**
 @file	Z:\Projects\wierd\source\gl\vao.cpp

 @brief	Implements the vertex array objects.
 */
#include "scene.h"
#include "vertexinput.h"

namespace Scene {

size_t VertexInput::getElementSize( const VinElement& element ) {
	switch( element.type ) {
	case VT_FLOAT4:   return sizeof(float)*4;
	case VT_FLOAT3:   return sizeof(float)*3; 
	case VT_FLOAT2:   return sizeof(float)*2;
	case VT_FLOAT1:   return sizeof(float); 
	case VT_BYTEARGB: return sizeof(uint32_t); 
	case VT_SHORT2:   return sizeof(short)*2; 
	default:
		CORE_ASSERT( false && "Unknown vertex type" );
		return 0;
	}
}

uint32_t VertexInput::getElementItemCount( const VinElement& element ) {
	switch( element.type ) {
	case VT_FLOAT4:   return 4;
	case VT_FLOAT3:   return 3; 
	case VT_FLOAT2:   return 2;
	case VT_FLOAT1:   return 1; 
	case VT_BYTEARGB: return 4; 
	case VT_SHORT2:   return 2; 
	default:
		CORE_ASSERT( false && "Unknown vertex type" );
		return 0;
	}
}


size_t VertexInput::getVertexSize( int numElements, const VinElement *type ) {
	size_t offset = 0;
	for( int i=0; i < numElements; ++i ) {
		offset += getElementSize( *type );
		++type;
	}
	return offset;
}

std::string VertexInput::genEleString( int numElements, const VinElement *type ) {
	std::string cacheName = "_";
	for( int i=0; i < numElements; ++i ) {

		const auto ourtype = type[i];

		switch( ourtype.usage ) {
		case VE_POSITION:	cacheName += "POS"; break;
		case VE_NORMAL:		cacheName += "NORM"; break;
		case VE_BINORMAL:	cacheName += "BINORM"; break;
		case VE_TANGENT:	cacheName += "TAN"; break;
		case VE_TEXCOORD0:	cacheName += "TC0"; break;
		case VE_TEXCOORD1:	cacheName += "TC1"; break;
		case VE_TEXCOORD2:	cacheName += "TC2"; break;
		case VE_TEXCOORD3:	cacheName += "TC3"; break;
		case VE_COLOUR0:	cacheName += "COL0"; break;
		case VE_COLOUR1:	cacheName += "COL1"; break;
		default:
			CORE_ASSERT( false && "Unknown vertex usage" );
		}

		switch( ourtype.type )
		{
		case VT_FLOAT4:		cacheName += "f4_"; break;
		case VT_FLOAT3:		cacheName += "f3_"; break;
		case VT_FLOAT2:		cacheName += "f2_"; break;
		case VT_FLOAT1:		cacheName += "f1_"; break;
		case VT_BYTEARGB:	cacheName += "bargb_"; break;
		case VT_SHORT2:		cacheName += "s2_"; break;
		default:
			CORE_ASSERT( false && "Unknown vertex type" );
		}
	}

	return cacheName;
}

}