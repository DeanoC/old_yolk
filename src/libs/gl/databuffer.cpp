/**
 @file	Z:\Projects\wierd\source\gl\databuffer.cpp

 @brief	Implements the data buffer objects.
 */
#include "ogl.h"
#include "databuffer.h"

#define BUFFER_OLD				0
#define BUFFER_DSA				1

#define BUFFER_METHOD			BUFFER_DSA
#define BUFFER_RANGE_ALWAYS


namespace {
	// map DATA_BUFFER_TYPE to GL
	static const uint32_t DBT_Map[] = {
		GL_ARRAY_BUFFER, 										// DBT_VERTEX 
		GL_ELEMENT_ARRAY_BUFFER, 								// DBT_INDEX
		GL_UNIFORM_BUFFER,										// DBT_CONSTANTS
		GL_ATOMIC_COUNTER_BUFFER,								// DBT_ATOMIC_COUNTERS
		GL_TEXTURE_BUFFER, 										// DBT_TEXTURE
		GL_TRANSFORM_FEEDBACK_BUFFER,							// DBT_GENERAL
	};
	
	// map DATA_BUFFER_MAP_ACCESS to GL
	static const uint32_t DBMA_Map[] = {
		GL_MAP_READ_BIT,										// DBMA_READ_ONLY
		GL_MAP_WRITE_BIT,										// DBMA_WRITE_ONLY
		GL_MAP_WRITE_BIT | GL_MAP_READ_BIT,						// DBMA_READ_WRITE
	};

	// map DATA_BUFFER_MAP_FLAGS to GL
	static const uint32_t DBMF_Map[] = {
		0,														// DBMF_NONE
		GL_MAP_INVALIDATE_BUFFER_BIT,							// DBMF_DISCARD
		GL_MAP_UNSYNCHRONIZED_BIT,								// DBMF_UNSYNC
	};

}

namespace Gl {

void* DataBuffer::map( Scene::DATA_BUFFER_MAP_ACCESS _access, Scene::DATA_BUFFER_MAP_FLAGS _flags, size_t offset, size_t bytes ) {
	GL_CHECK

	const auto access = DBMA_Map[ _access ];
	const auto flags = DBMF_Map[ _flags ];

	if( bytes == 0 ) {
#if !defined( BUFFER_RANGE_ALWAYS )
		GLbitfield mbaccess;
		if ( (access & (GL_MAP_WRITE_BIT|GL_MAP_READ_BIT)) == (GL_MAP_WRITE_BIT|GL_MAP_READ_BIT) ) {
			mbaccess = GL_READ_WRITE;
		} else if( access & MA_READ_ONLY ) {
			mbaccess = GL_READ_ONLY;
		} else if( access & MA_WRITE_ONLY ) {
			mbaccess = GL_WRITE_ONLY;
		} else {
			CORE_ASSERT( false );
		}

#if BUFFER_METHOD == BUFFER_OLD
		glBindBufferBase( DBT_Map[ type ], 14, name );
		GL_CHECK
		auto ret = glMapBuffer( DBT_Map[ type ], mbaccess );		
		GL_CHECK
		glBindBufferBase( DBT_Map[ type ], 14, 0 );
		GL_CHECK
#else
		auto ret = glMapNamedBufferEXT( name, mbaccess );
		GL_CHECK
#endif
		return ret;
#else
		bytes = size;
		offset = 0;
#endif
	}
	GLbitfield gflags = (GLbitfield) access;
	GLbitfield mflags = (GLbitfield) flags;
	
	if( flags & Scene::DBMF_DISCARD ) {
		if( bytes != size ) {
			// swap whole buffer bit for ranged bit
			mflags = (flags & ~GL_MAP_INVALIDATE_BUFFER_BIT) | GL_MAP_INVALIDATE_RANGE_BIT;
		}
	}

	bytes = (size_t)Core::alignTo( bytes, DataBuffer::MIN_BUFFER_SIZE );

#if BUFFER_METHOD == BUFFER_OLD
	glBindBufferBase( DBT_Map[ type ], 14, name );
	GL_CHECK
	auto ret = glMapBufferRange( DBT_Map[ type ], offset, size, gflags | mflags );		
	GL_CHECK
	glBindBufferBase( DBT_Map[ type ], 14, 0 );
	GL_CHECK
#else
	auto ret = glMapNamedBufferRangeEXT( name, offset, size, gflags | mflags );
	GL_CHECK
#endif
	return ret;
}

void DataBuffer::unmap() {
#if BUFFER_METHOD == BUFFER_OLD
	glBindBufferBase( DBT_Map[ type ], 14, name );
	GL_CHECK
	glUnmapBuffer( DBT_Map[ type ] );
	GL_CHECK
	glBindBufferBase( DBT_Map[ type ], 14, 0 );
	GL_CHECK
#else
	glUnmapNamedBufferEXT( name );
	GL_CHECK
#endif
	
}

DataBuffer* DataBuffer::internalCreate(	const Core::ResourceHandleBase* baseHandle, 
											const char* name, 
											const DataBuffer::CreationStruct* creation ) {
	using namespace Scene;

	DataBuffer* dbuffer = CORE_NEW DataBuffer();
	dbuffer->generateName( MNT_DATA_BUFFER );
//	dbuffer->size = creation->size;
	dbuffer->size = (size_t)Core::alignTo( creation->size, DataBuffer::MIN_BUFFER_SIZE);
	CORE_ASSERT( dbuffer->size >= creation->size );
	dbuffer->type = creation->type;

	// immutable are pre-filled and never changed by cpu or gpu
	if( creation->flags & DBCF_IMMUTABLE ) {
#if BUFFER_METHOD == BUFFER_OLD
		glBindBuffer( DBT_Map[ dbuffer->type ], dbuffer->getName() );
		GL_CHECK
		glBufferData( DBT_Map[ dbuffer->type ], dbuffer->size, creation->data,  GL_STATIC_DRAW );
		GL_CHECK
		glBindBuffer( DBT_Map[ dbuffer->type ], 0 );
		GL_CHECK
#else
		glNamedBufferDataEXT( dbuffer->getName(), dbuffer->size, creation->data,  GL_STATIC_DRAW );
		GL_CHECK
#endif
		return dbuffer;
	}

	GLenum usage = GL_STATIC_DRAW;
	if( creation->flags & DBCF_CPU_READBACK ) {
		if( creation->flags & DBCF_CPU_UPDATES ) {
			usage = GL_STREAM_READ; // is this right??
			// issue warning that read-back and update on the same buffer is 
			// likely a performance issue
			LOG(INFO) << name << " : DBCF_CPU_READBACK and DBCF_CPU_UPDATES on the same buffer may be slow";
		} else {
			if( creation->flags & DBCF_ONCE ) {
				usage = GL_STATIC_READ; // once only read-back
			} else if( creation->flags & DBCF_INTERMITANT ) {
				usage = GL_DYNAMIC_READ; // read-backs will occur every now and again
			} else usage = GL_STREAM_READ; // regular CPU read-backs
		}
	} else if( creation->flags & DBCF_CPU_UPDATES ) {
		if( creation->flags & DBCF_ONCE ) {
			usage = GL_STATIC_DRAW; // once only CPU update
		} else if( creation->flags & DBCF_INTERMITANT ) {
			usage = GL_DYNAMIC_DRAW; // CPU updates will occur every now and again
		} else usage = GL_STREAM_DRAW;	// regular CPU updates
	} else if( creation->flags & DBCF_GPU_COPY ) {
		if( creation->flags & DBCF_ONCE ) {
			usage = GL_STATIC_COPY; // once only GPU update
		} else if( creation->flags & DBCF_INTERMITANT ) {
			usage = GL_DYNAMIC_COPY; // GPU updates will occur every now and again
		} else usage = GL_STREAM_COPY; // regular GPU updates
	}

#if BUFFER_METHOD == BUFFER_OLD
	glBindBuffer( DBT_Map[ dbuffer->type ], dbuffer->getName() );
	GL_CHECK
	glBufferData( DBT_Map[ dbuffer->type ], dbuffer->size, NULL, usage );
	GL_CHECK
	glBindBuffer( DBT_Map[ dbuffer->type ], 0 );
	GL_CHECK
#else
	glNamedBufferDataEXT( dbuffer->getName(), dbuffer->size, NULL, usage );
	GL_CHECK
#endif
	return dbuffer;
}

}
