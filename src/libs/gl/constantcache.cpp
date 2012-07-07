//!-----------------------------------------------------
//!
//! \file constantcache.cpp
//!
//!
//!-----------------------------------------------------

#include "gl.h"
#include "databuffer.h"
#include "gpu_constants.h"
#include "program.h"
#include "cl/platform.h"
#include "constantcache.h"

namespace {

const int varFreq[Gl::CVN_NUM_CONSTANTS] = {
	Gl::CF_PER_VIEWS,		//	VIEW,
	Gl::CF_PER_VIEWS,		//	VIEW_INVERSE,
	Gl::CF_PER_VIEWS,		//	VIEW_IT,
	Gl::CF_PER_PIPELINE,	//	PROJ,
	Gl::CF_PER_PIPELINE,	//	PROJ_INVERSE,
	Gl::CF_PER_PIPELINE,	//	PROJ_IT,
	Gl::CF_PER_VIEWS,		//	VIEW_PROJ,
	Gl::CF_PER_VIEWS,		//	VIEW_PROJ_INVERSE,
	Gl::CF_PER_VIEWS,		//	VIEW_PROJ_IT,
	Gl::CF_STD_OBJECT,		//	WORLD,
	Gl::CF_STD_OBJECT,		//	WORLD_INVERSE,
	Gl::CF_STD_OBJECT,		//	WORLD_IT,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW_INVERSE,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW_IT,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ_INVERSE,
	Gl::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ_IT,
	Gl::CF_STD_OBJECT,		//	PREV_WORLD_VIEW_PROJ
	0,						//	CVN_NUM_MATRICES
	Gl::CF_PER_FRAME,		//	FRAMECOUNT
	Gl::CF_PER_TARGETS,		//	TARGET_DIMS
	Gl::CF_STATIC,			//	DUMMY
	Gl::CF_PER_PIPELINE,	//	ZPLANES
	Gl::CF_PER_PIPELINE,	//	FOV
};

const uint32_t varsPerBlock[ Gl::CF_USER_BLOCKS ] = {
	1,		// CF_STATIC,
	1,		// CF_PER_FRAME
	5,		// CF_PER_PIPELINE
	6,		// CF_PER_VIEWS
	1,		// CF_PER_TARGETS
	10,		// CF_STD_OBJECT
};

const Gl::ConstantCache::CachedBitFlags varBitsPerBlock[ Gl::CF_USER_BLOCKS ] = {
	BIT64(Gl::CVN_DUMMY),
	BIT64(Gl::CVN_FRAMECOUNT),
	BIT64(Gl::CVN_PROJ) | BIT64(Gl::CVN_PROJ_INVERSE) | BIT64(Gl::CVN_PROJ_IT) | BIT64(Gl::CVN_ZPLANES) | BIT64(Gl::CVN_FOV),
	BIT64(Gl::CVN_VIEW) | BIT64(Gl::CVN_VIEW_INVERSE) | BIT64(Gl::CVN_VIEW_IT) | BIT64(Gl::CVN_VIEW_PROJ) | BIT64(Gl::CVN_VIEW_PROJ_INVERSE) | BIT64(Gl::CVN_VIEW_PROJ_IT),
	BIT64(Gl::CVN_TARGET_DIMS),
	BIT64(Gl::CVN_WORLD) | BIT64(Gl::CVN_WORLD_INVERSE) | BIT64(Gl::CVN_WORLD_IT) | BIT64(Gl::CVN_WORLD_VIEW) | BIT64(Gl::CVN_WORLD_VIEW_INVERSE) | BIT64(Gl::CVN_WORLD_VIEW_IT) 
		| BIT64(Gl::CVN_WORLD_VIEW_PROJ) | BIT64(Gl::CVN_WORLD_VIEW_PROJ_INVERSE) | BIT64(Gl::CVN_WORLD_VIEW_PROJ_IT) | BIT64(Gl::CVN_PREV_WORLD_VIEW_PROJ),
};

const size_t offsetInBlocks[ Gl::CVN_NUM_CONSTANTS ] = {
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixView ),							// VIEW
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewInverse ),					// VIEW_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewIT ),							// VIEW_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerPipeline, matrixProjection ),					// PROJ
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerPipeline, matrixProjectionInverse ),			// PROJ_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerPipeline, matrixProjectionIT ),				// PROJ_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjection ),					// VIEW_PROJ
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjectionInverse ),			// VIEW_PROJ_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjectionIT ),				// VIEW_PROJ_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorld ),							// WORLD
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldInverse ),					// WORLD_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldIT ),						// WORLD_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldView ),						// WORLD_VIEW
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewInverse ),				// WORLD_VIEW_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewIT ),					// WORLD_VIEW_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjection ),			// WORLD_VIEW_PROJ
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjectionInverse ),	// WORLD_VIEW_PROJ_INVERSE
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjectionIT ),			// WORLD_VIEW_PROJ_IT
	WIERD_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixPreviousWorldViewProjection ),	// PREV_WORLD_VIEW_PROJ 
	0,					//		CVN_NUM_MATRICES
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerFrame, frameCount ),							// FRAMERATE
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerTargets, targetDims),							// TARGET_DIMS
	WIERD_GL_GET_OFFSET_IN_BLOCK( Static, dummy ),									// DUMMY 
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerPipeline, zPlanes ),							// ZPLANES
	WIERD_GL_GET_OFFSET_IN_BLOCK( PerPipeline, fov ),								// FOV

};

const size_t sizeofInBlock[ Gl::CVN_NUM_CONSTANTS ] = {
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixView ),							// VIEW
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewInverse ),						// VIEW_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewIT ),							// VIEW_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerPipeline, matrixProjection ),					// PROJ
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerPipeline, matrixProjectionInverse ),			// PROJ_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerPipeline, matrixProjectionIT ),					// PROJ_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjection ),					// VIEW_PROJ
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjectionInverse ),			// VIEW_PROJ_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjectionIT ),				// VIEW_PROJ_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorld ),							// WORLD
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldInverse ),					// WORLD_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldIT ),						// WORLD_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldView ),						// WORLD_VIEW
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewInverse ),				// WORLD_VIEW_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewIT ),					// WORLD_VIEW_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjection ),			// WORLD_VIEW_PROJ
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjectionInverse ),		// WORLD_VIEW_PROJ_INVERSE
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjectionIT ),			// WORLD_VIEW_PROJ_IT
	WIERD_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixPreviousWorldViewProjection ),	// PREV_WORLD_VIEW_PROJ 
	0,					//		CVN_NUM_MATRICES
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerFrame, frameCount ),							// FRAMERATE
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerTargets, targetDims),							// TARGET_DIMS
	WIERD_GL_GET_SIZEOF_PRG_VAR( Static, dummy ),									// DUMMY 
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerPipeline, zPlanes ),							// ZPLANES
	WIERD_GL_GET_SIZEOF_PRG_VAR( PerPipeline, fov ),								// FOV

};

const int NUM_MATRIX_BITS = (1ULL << Gl::CVN_NUM_MATRICES);

}

namespace Gl {

int ConstantCache::s_cacheCount = 0;

ConstantCache::ConstantCache() :
	cachedFlags( 0 ),
	gpuHasBlocks(0) {

	blockHandles.reset( CORE_NEW_ARRAY DataBufferHandlePtr[CF_USER_BLOCKS] );
	clBlockHandles.reset( CORE_NEW_ARRAY Cl::BufferHandlePtr[CF_USER_BLOCKS] );

	auto contextCl = Cl::Platform::get()->getPrimaryContext().get();

	size_t blockSizes[ CF_USER_BLOCKS ] = {
		sizeof(Gl::GPUConstants::Static),
		sizeof(Gl::GPUConstants::PerFrame),
		sizeof(Gl::GPUConstants::PerPipeline),
		sizeof(Gl::GPUConstants::PerViews),
		sizeof(Gl::GPUConstants::PerTargets),
		sizeof(Gl::GPUConstants::StdObject),
	};

	char resourceName[256];
	for( int i = 0; i < CF_USER_BLOCKS; ++i ) {
		DataBuffer::CreationStruct cbcs = {
			DBCF_CPU_UPDATES, DBT_CONSTANTS, 
			blockSizes[i]
		};
		sprintf( resourceName, "_constantCache%i_%i", i, s_cacheCount );
		blockHandles[i] = DataBufferHandle::create( resourceName, &cbcs );
/*		Cl::Buffer::CreationStruct clfbcs = {
			contextCl,
			(Cl::BUFFER_CREATION_FLAGS) (Cl::BCF_FROM_GL | Cl::BCF_KERNEL_WRITE | Cl::BCF_KERNEL_READ ),
			0, (void*) blockHandles[i]
		};
		sprintf( resourceName, "_cl_constantCache%i_%i", i, s_cacheCount );
		clBlockHandles[i] = Cl::BufferHandle::create( resourceName, &clfbcs );*/

	}
	s_cacheCount++;
}

ConstantCache::~ConstantCache() {
	for( int i = 0; i < CF_USER_BLOCKS; ++i ) {
//		clBlockHandles[i]->close();
		blockHandles[i]->close();
	}
}

void ConstantCache::changeObject(	const Math::Matrix4x4& prevWVPMatrix,
									const Math::Matrix4x4& worldMatrix ) {
	matrixCache[ CVN_PREV_WORLD_VIEW_PROJ ] = prevWVPMatrix;
	matrixCache[ CVN_WORLD ] = worldMatrix;
	cachedFlags &= ~(	
		BIT64(CVN_WORLD_INVERSE) | BIT64(CVN_WORLD_IT) |
		BIT64(CVN_WORLD_VIEW) | BIT64(CVN_WORLD_VIEW_INVERSE) | BIT64(CVN_WORLD_VIEW_IT) |
		BIT64(CVN_WORLD_VIEW_PROJ) | BIT64(CVN_WORLD_VIEW_PROJ_INVERSE) | BIT64(CVN_WORLD_VIEW_PROJ_IT)
	);
	cachedFlags |= BIT64(CVN_PREV_WORLD_VIEW_PROJ);
	cachedFlags |= BIT64(CVN_WORLD);
	gpuHasBlocks &= ~( BIT(CF_STD_OBJECT) );
}
void ConstantCache::setMatrixBypassCache( CONSTANT_VAR_NAME type, const Math::Matrix4x4& mat ) {
	matrixCache[ type ] = mat;

	cachedFlags |= BIT64(type);
	gpuHasBlocks &= ~( BIT(varFreq[type] ));

}
void ConstantCache::invalidCacheOfType( CONSTANT_VAR_NAME type ) {
	cachedFlags &= ~(BIT64(type));
	gpuHasBlocks &= ~( BIT(varFreq[type] ));
}

const Math::Matrix4x4& ConstantCache::getMatrix( CONSTANT_VAR_NAME type ) const {
	using namespace Math;
	CORE_ASSERT( type < CVN_NUM_MATRICES );

	// if the cache is valid just return
	if( cachedFlags & BIT64(type) ) {
		return matrixCache[ type ];
	} 

	// else recompute
	cachedFlags |= BIT64(type);
	switch( type ) {
	case CVN_VIEW:
		return matrixCache[ type ];
	case CVN_VIEW_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_VIEW) );
	case CVN_VIEW_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_VIEW_INVERSE) );
	case CVN_PROJ:
		return matrixCache[ type ];
	case CVN_PROJ_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_PROJ) );
	case CVN_PROJ_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_PROJ_INVERSE) );
	case CVN_VIEW_PROJ:
		return matrixCache[ type ] = getMatrix(CVN_VIEW) * getMatrix(CVN_PROJ) ;
	case CVN_VIEW_PROJ_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_VIEW_PROJ) );
	case CVN_VIEW_PROJ_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_VIEW_PROJ_INVERSE) );
	case CVN_WORLD:
		return matrixCache[ type ];
	case CVN_WORLD_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD) );
	case CVN_WORLD_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_INVERSE) );
	case CVN_WORLD_VIEW:
		return matrixCache[ type ] = getMatrix(CVN_WORLD) * getMatrix(CVN_VIEW);	
	case CVN_WORLD_VIEW_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD_VIEW) );
	case CVN_WORLD_VIEW_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_VIEW_INVERSE) );
	case CVN_WORLD_VIEW_PROJ:
		return matrixCache[ type ] = getMatrix(CVN_WORLD_VIEW) * getMatrix(CVN_PROJ);
	case CVN_WORLD_VIEW_PROJ_INVERSE:
		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD_VIEW_PROJ) );
	case CVN_WORLD_VIEW_PROJ_IT:
		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_VIEW_PROJ_INVERSE) );
	case CVN_PREV_WORLD_VIEW_PROJ:
		return matrixCache[ type ];
	default:
		CORE_ASSERT( false && ("Invalid matrix type") );
		return matrixCache[ 0 ];
	};
}

void ConstantCache::getUIVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const {
	switch( type ) {
	case CVN_FRAMECOUNT:
	case CVN_TARGET_DIMS:
	case CVN_DUMMY: // though float you can access as uint32_t (makes uploader simplier...
	case CVN_ZPLANES:
	case CVN_FOV:
		memcpy( out4, &vectorCache[(type-CVN_NUM_MATRICES)*16], 16 );
		break;
	default:
		CORE_ASSERT( false && ("Invalid matrix type") );
	}
}

const Math::Vector4& ConstantCache::getVector( CONSTANT_VAR_NAME type ) const {

	switch( type ) {
	case CVN_DUMMY:
	case CVN_ZPLANES:
	case CVN_FOV:
		return *(Math::Vector4*)&vectorCache[(type-CVN_NUM_MATRICES)*16];
	default:
		CORE_ASSERT( false && ("Invalid matrix type") );
		return *(Math::Vector4*)&vectorCache[ 0 ];
	}
}

void ConstantCache::setUIVector( CONSTANT_VAR_NAME type, const uint32_t* in4 ) {
	switch( type ) {
	case CVN_FRAMECOUNT:
	case CVN_TARGET_DIMS:
	case CVN_DUMMY: // though float you can access as uint32_t (makes uploader simplier...
	case CVN_ZPLANES:
	case CVN_FOV:
		memcpy( &vectorCache[(type-CVN_NUM_MATRICES)*16], in4, 16 );
		break;
	default:
		CORE_ASSERT( false && ("Invalid matrix type") );
	}
}

void ConstantCache::updateGPUBlock( CONSTANT_FREQ_BLOCKS block ) const {
	DataBufferPtr db = blockHandles[block]->acquire();
	void* buffer = db->map( DataBuffer::MA_WRITE_ONLY, DataBuffer::MF_DISCARD, 0, 0 );
	int varsToSet = varsPerBlock[block];
	int count = 0;
	while( varsToSet > 0 ) {
		// TODO better than naive bit checking
		if( varBitsPerBlock[block] & BIT64(count) ) {
			if( count < CVN_NUM_MATRICES ) {
				Math::Matrix4x4 mat = getMatrix((CONSTANT_VAR_NAME)count); // compute if required
				memcpy( ((char*)buffer) + offsetInBlocks[ count ], 
						&mat, 
						sizeofInBlock[ count ] );
			} else {
				uint32_t uiArray[4];
				getUIVector( (CONSTANT_VAR_NAME)count, uiArray );
				memcpy( ((char*)buffer) + offsetInBlocks[ count ], 
						uiArray, 
						sizeofInBlock[ count ] );
			}
			--varsToSet;
		}
		count++;
	}
	db->unmap();
	gpuHasBlocks |= BIT(block);
}

void ConstantCache::updateGPU( const ProgramPtr prg ) {
	if( camera && camera->getCounter() != s_cacheCount ) {

		s_cacheCount = camera->getCounter();

		matrixCache[ CVN_VIEW ] = camera->getView();
		matrixCache[ CVN_PROJ ] = camera->getProjection();
		float zPlanes[4] = { camera->getZNear(), camera->getZFar(), 0, 0 };
		setUIVector( CVN_ZPLANES, (const uint32_t*) zPlanes );
		float fov[4] = { camera->getXScale(), camera->getYScale(), 0, 0 };
		setUIVector( CVN_FOV, (const uint32_t*) fov );

		// reset cached flags
		cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_VIEW ] ] );
		cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_PROJ ] ] );
		cachedFlags &= ~(varBitsPerBlock [ CF_STD_OBJECT ] );

		cachedFlags |= BIT64(CVN_VIEW) | BIT64(CVN_PROJ) | BIT64(CVN_ZPLANES);
		gpuHasBlocks &= ~ ( BIT(varFreq[ CVN_VIEW ]) | BIT(varFreq[ CVN_PROJ ]) | BIT(CF_STD_OBJECT) );
	}

	if( prg && (gpuHasBlocks & prg->getUsedBuffers()) == prg->getUsedBuffers() ) {
		// upto date, nothing to do
		return;
	}

	// what needs updating?
	uint32_t needsUpdating = (~gpuHasBlocks) & (prg ? prg->getUsedBuffers() : ~0);

	for( int i = 0; i < CF_USER_BLOCKS; ++i ) {
		if( needsUpdating & BIT(i) ) {
			updateGPUBlock( (CONSTANT_FREQ_BLOCKS)i );
		}
	}
}
void ConstantCache::updateGPUObjectOnly() {
	updateGPUBlock( CF_STD_OBJECT );
}

void ConstantCache::bind() {
	for( int i = 0; i < CF_USER_BLOCKS; ++i ) {
		DataBufferPtr db = blockHandles[i]->acquire();
		glBindBufferBase( GL_UNIFORM_BUFFER, i, db->getName() );
		GL_CHECK
	}
}

void ConstantCache::unbind() {
//	for( int i = 0; i < CF_USER_BLOCKS; ++i ) {
//		glBindBufferBase( GL_UNIFORM_BUFFER, i, 0 );
//	}
}

const Cl::BufferHandlePtr ConstantCache::getClBlock( CONSTANT_FREQ_BLOCKS block ) const { 
	updateGPUBlock( block );

	return clBlockHandles[ block ]; 
}

} // end Gl namespace
