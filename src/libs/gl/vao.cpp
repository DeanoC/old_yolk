/**
 @file	Z:\Projects\wierd\source\gl\vao.cpp

 @brief	Implements the vertex array objects.
 */
#include "gl.h"
#include "vao.h"

namespace Gl {
size_t Vao::getElementSize( const VaoElement element ) {
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

uint32_t Vao::getElementItemCount( const VaoElement element ) {
	switch( element.type ) {
	case VT_FLOAT4:   return 4;
	case VT_FLOAT3:   return 3; 
	case VT_FLOAT2:   return 2;
	case VT_FLOAT1:   return 1; 
	case VT_BYTEARGB: return GL_BGRA; // special for D3D format colours 
	case VT_SHORT2:   return 2; 
	default:
		CORE_ASSERT( false && "Unknown vertex type" );
		return 0;
	}
}


size_t Vao::getVertexSize( int numElements, const VaoElement *type ) {
	size_t offset = 0;
	for( int i=0; i < numElements; ++i ) {
		offset += Vao::getElementSize( *type );
		++type;
	}
	return offset;
}

std::string Vao::genEleString( int numElements, const VaoElement *type ) {
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

GLenum Vao::getElementGlType( const VaoElement element ) {
	switch( element.type ) {
	case VT_FLOAT4:   return GL_FLOAT;
	case VT_FLOAT3:   return GL_FLOAT; 
	case VT_FLOAT2:   return GL_FLOAT;
	case VT_FLOAT1:   return GL_FLOAT; 
	case VT_BYTEARGB: return GL_UNSIGNED_BYTE; 
	case VT_SHORT2:   return GL_UNSIGNED_SHORT; 
	default:
		CORE_ASSERT( false && "Unknown vertex type" );
		return 0;
	}
}


Vao::Vao() {
	generateName( MNT_VERTEX_ARRAY_OBJECT );
}

Vao* Vao::internalCreate(const Core::ResourceHandleBase* baseHandle, 
							const char* pName, const Vao::CreationStruct* cr ) {
	CORE_ASSERT( cr->elementCount >= 0 );

	DataBufferPtr dbs[ MAX_ELEMENT_COUNT ];
	size_t streamOffset[ MAX_ELEMENT_COUNT + 1] = { 0 };
	size_t streamStride[ MAX_ELEMENT_COUNT ] = { 0 };

	// once first to calc auto strides and offsets also debug checks are here
	for( int i = 0; i < cr->elementCount; ++i ) {
		CORE_ASSERT( cr->data[i].buffer != NULL );
		if( cr->data[i].stride == AUTO_STRIDE ) {
			streamStride[ cr->data[i].stream ] += getElementSize( cr->elements[i] );
		} else {
			// if multiple elements to the same stride set different strides, its undefined
			streamStride[ cr->data[i].stream ] = cr->data[i].stride;
		}

		size_t offset = 0;
		if( cr->data[i].offset == AUTO_OFFSET ) {
			offset = streamOffset[ i ];
		} else {
			offset = cr->data[i].offset;
		}
		streamOffset[ i + 1 ] = offset + getElementSize( cr->elements[i] );
		dbs[i] = cr->data[i].buffer->tryAcquire();
		if( !dbs[i] ) {
			return nullptr;
		}		
	}

	Vao* vao = CORE_NEW Vao();
	for( int i = 0; i < cr->elementCount; ++i ) {

		DataBufferPtr db = dbs[i];
		
		CORE_ASSERT( db->getName() );
		glVertexArrayVertexAttribOffsetEXT( vao->getName(),
											db->getName(),
											cr->elements[i].usage, 
											getElementItemCount(cr->elements[i]), 
											getElementGlType(cr->elements[i]),
											GL_TRUE, // may change with more vertex formats
											streamStride[ cr->data[i].stream ],
											streamOffset[ i ] );

		glEnableVertexArrayAttribEXT( vao->getName(), cr->elements[i].usage );
	}

	return vao;
}

}
