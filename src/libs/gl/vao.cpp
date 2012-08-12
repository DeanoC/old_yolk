/**
 @file	Z:\Projects\wierd\source\gl\vao.cpp

 @brief	Implements the vertex array objects.
 */
#include "ogl.h"
#include "databuffer.h"
#include "vao.h"

namespace Gl {

GLenum Vao::getElementGlType( const Scene::VinElement element ) {
	using namespace Scene;
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
							const char* pName, const Scene::VertexInput::CreationStruct* cr ) {
	using namespace Scene;
	CORE_ASSERT( cr->elementCount >= 0 );

	DataBufferPtr dbs[ MAX_ELEMENT_COUNT ];
	size_t streamOffset[ MAX_ELEMENT_COUNT + 1] = { 0 };
	size_t streamStride[ MAX_ELEMENT_COUNT ] = { 0 };

	// once first to calc auto strides and offsets also debug checks are here
	for( int i = 0; i < cr->elementCount; ++i ) {
		CORE_ASSERT( cr->data[i].buffer != NULL );
		if( cr->data[i].stride == VI_AUTO_STRIDE ) {
			streamStride[ cr->data[i].stream ] += getElementSize( cr->elements[i] );
		} else {
			// if multiple elements to the same stride set different strides, its undefined
			streamStride[ cr->data[i].stream ] = cr->data[i].stride;
		}

		size_t offset = 0;
		if( cr->data[i].offset == VI_AUTO_OFFSET ) {
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

		auto db = std::static_pointer_cast<Gl::DataBuffer>( dbs[i] );

		// handle special BGRA special case, probably should be fixed in wobs tbh...
		const auto elC = (cr->elements[i].type == VT_BYTEARGB) ? GL_BGRA : getElementItemCount(cr->elements[i]);
		
		CORE_ASSERT( db->getName() );
		glVertexArrayVertexAttribOffsetEXT( vao->getName(),
											db->getName(),
											cr->elements[i].usage, 
											elC,
											getElementGlType(cr->elements[i]),
											GL_TRUE, // may change with more vertex formats
											streamStride[ cr->data[i].stream ],
											streamOffset[ i ] );

		glEnableVertexArrayAttribEXT( vao->getName(), cr->elements[i].usage );
	}

	return vao;
}

}
