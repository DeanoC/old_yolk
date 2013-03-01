//!-----------------------------------------------------
//!
//! \file constantcache.cpp
//!
//!
//!-----------------------------------------------------

#include "scene.h"
#include "databuffer.h"
#include "gpu_constants.h"
#include "program.h"
//#include "cl/platform.h"
#include "constantcache.h"

namespace {

const int varFreq[Scene::CVN_NUM_CONSTANTS] = {
	Scene::CF_PER_VIEWS,		//	VIEW,
	Scene::CF_PER_VIEWS,		//	VIEW_INVERSE,
	Scene::CF_PER_VIEWS,		//	VIEW_IT,
	Scene::CF_PER_VIEWS,		//	PROJ,
	Scene::CF_PER_VIEWS,		//	PROJ_INVERSE,
	Scene::CF_PER_VIEWS,		//	PROJ_IT,
	Scene::CF_PER_VIEWS,		//	VIEW_PROJ,
	Scene::CF_PER_VIEWS,		//	VIEW_PROJ_INVERSE,
	Scene::CF_PER_VIEWS,		//	VIEW_PROJ_IT,
	Scene::CF_STD_OBJECT,		//	WORLD,
	Scene::CF_STD_OBJECT,		//	WORLD_INVERSE,
	Scene::CF_STD_OBJECT,		//	WORLD_IT,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW_INVERSE,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW_IT,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ_INVERSE,
	Scene::CF_STD_OBJECT,		//	WORLD_VIEW_PROJ_IT,
	Scene::CF_STD_OBJECT,		//	PREV_WORLD_VIEW_PROJ
	Scene::CF_STD_OBJECT,		//	USER_MATRIX_0,
	Scene::CF_STD_OBJECT,		//	USER_MATRIX_1
	0,							//	CVN_NUM_MATRICES
	Scene::CF_PER_FRAME,		//	FRAMECOUNT
	Scene::CF_PER_TARGETS,		//	TARGET_DIMS
	Scene::CF_PER_PIPELINE,		//	ZPLANES
	Scene::CF_PER_PIPELINE,		//	FOV
	Scene::CF_PER_MATERIAL,		//	MATERIAL_INDEX
	Scene::CF_PER_PIPELINE,		//	NUMLIGHTS
};

const uint32_t varsPerBlock[ Scene::CF_NUM_BLOCKS ] = {
	0,		// CF_STATIC,
	1,		// CF_PER_FRAME
	1,		// CF_PER_PIPELINE
	11,		// CF_PER_VIEWS
	1,		// CF_PER_TARGETS
	12,		// CF_STD_OBJECT
	1,		// CF_PER_MATERIAL
};

const Scene::ConstantCache::CachedBitFlags varBitsPerBlock[ Scene::CF_NUM_BLOCKS ] = {
	0,
	BIT64(Scene::CVN_FRAMECOUNT),
	BIT64(Scene::CVN_LIGHT_COUNTS),
	BIT64(Scene::CVN_PROJ) | BIT64(Scene::CVN_PROJ_INVERSE) | BIT64(Scene::CVN_PROJ_IT) | BIT64(Scene::CVN_ZPLANES) | BIT64(Scene::CVN_FOV) |
		BIT64(Scene::CVN_VIEW) | BIT64(Scene::CVN_VIEW_INVERSE) | BIT64(Scene::CVN_VIEW_IT) | BIT64(Scene::CVN_VIEW_PROJ) | BIT64(Scene::CVN_VIEW_PROJ_INVERSE) | 
		BIT64(Scene::CVN_VIEW_PROJ_IT),
	BIT64(Scene::CVN_TARGET_DIMS),
	BIT64(Scene::CVN_WORLD) | BIT64(Scene::CVN_WORLD_INVERSE) | BIT64(Scene::CVN_WORLD_IT) | BIT64(Scene::CVN_WORLD_VIEW) | BIT64(Scene::CVN_WORLD_VIEW_INVERSE) | 
		BIT64(Scene::CVN_WORLD_VIEW_IT) | BIT64(Scene::CVN_WORLD_VIEW_PROJ) | BIT64(Scene::CVN_WORLD_VIEW_PROJ_INVERSE) | BIT64(Scene::CVN_WORLD_VIEW_PROJ_IT) | 
		BIT64(Scene::CVN_PREV_WORLD_VIEW_PROJ) | BIT64(Scene::CVN_USER_MATRIX_0) | BIT64(Scene::CVN_USER_MATRIX_1),
	BIT64(Scene::CVN_MATERIAL_INDEX),
};

const size_t offsetInBlocks[ Scene::CVN_NUM_CONSTANTS ] = {
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixView ),							// VIEW
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewInverse ),						// VIEW_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewIT ),							// VIEW_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixProjection ),						// PROJ
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixProjectionInverse ),				// PROJ_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixProjectionIT ),					// PROJ_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjection ),					// VIEW_PROJ
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjectionInverse ),			// VIEW_PROJ_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, matrixViewProjectionIT ),				// VIEW_PROJ_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorld ),							// WORLD
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldInverse ),					// WORLD_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldIT ),						// WORLD_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldView ),						// WORLD_VIEW
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewInverse ),				// WORLD_VIEW_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewIT ),					// WORLD_VIEW_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjection ),			// WORLD_VIEW_PROJ
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjectionInverse ),		// WORLD_VIEW_PROJ_INVERSE
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixWorldViewProjectionIT ),			// WORLD_VIEW_PROJ_IT
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixPreviousWorldViewProjection ),	// PREV_WORLD_VIEW_PROJ 
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixUser0 ),							// USER_MATRIX_0
	YOLK_GL_GET_OFFSET_IN_BLOCK( StdObject, matrixUser1 ),							// USER_MATRIX_1
	0,					//		CVN_NUM_MATRICES
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerFrame, frameCount ),							// FRAMERATE
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerTargets, targetDims),							// TARGET_DIMS
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, zPlanes ),								// ZPLANES
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerViews, fov ),									// FOV
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerMaterial, materialIndex ),						// MATERIAL_INDEX
	YOLK_GL_GET_OFFSET_IN_BLOCK( PerPipeline, lightCounts ),						// LIGHT_COUNTS

};

const size_t sizeofInBlock[ Scene::CVN_NUM_CONSTANTS ] = {
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixView ),							// VIEW
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewInverse ),					// VIEW_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewIT ),						// VIEW_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixProjection ),					// PROJ
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixProjectionInverse ),			// PROJ_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixProjectionIT ),					// PROJ_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjection ),				// VIEW_PROJ
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjectionInverse ),		// VIEW_PROJ_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, matrixViewProjectionIT ),				// VIEW_PROJ_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorld ),						// WORLD
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldInverse ),				// WORLD_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldIT ),						// WORLD_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldView ),					// WORLD_VIEW
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewInverse ),			// WORLD_VIEW_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewIT ),					// WORLD_VIEW_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjection ),			// WORLD_VIEW_PROJ
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjectionInverse ),	// WORLD_VIEW_PROJ_INVERSE
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixWorldViewProjectionIT ),		// WORLD_VIEW_PROJ_IT
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixPreviousWorldViewProjection ),	// PREV_WORLD_VIEW_PROJ 
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixUser0 ),						// USER_MATRIX_0
	YOLK_GL_GET_SIZEOF_PRG_VAR( StdObject, matrixUser1 ),						// USER_MATRIX_1
	0,					//		CVN_NUM_MATRICES
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerFrame, frameCount ),							// FRAMERATE
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerTargets, targetDims),						// TARGET_DIMS
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, zPlanes ),							// ZPLANES
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerViews, fov ),								// FOV
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerMaterial, materialIndex ),					// MATERIAL_INDEX
	YOLK_GL_GET_SIZEOF_PRG_VAR( PerPipeline, lightCounts ),						// LIGHT_COUNTS

};

const int NUM_MATRIX_BITS = (1ULL << Scene::CVN_NUM_MATRICES);

}

namespace Scene {

ConstantCache::ConstantCache() :
	cachedFlags( 0 ),
	gpuHasBlocks(0) {

	blockHandles.reset( CORE_NEW_ARRAY DataBufferHandlePtr[CF_NUM_BLOCKS] );

	size_t blockSizes[ CF_NUM_BLOCKS ] = {
		sizeof(GPUConstants::Static),
		sizeof(GPUConstants::PerFrame),
		sizeof(GPUConstants::PerPipeline),
		sizeof(GPUConstants::PerViews),
		sizeof(GPUConstants::PerTargets),
		sizeof(GPUConstants::StdObject),	
		sizeof(GPUConstants::PerMaterial),
	};

	char resourceName[256];
	for( int i = 0; i < CF_NUM_BLOCKS; ++i ) {
		DataBuffer::CreationInfo cbcs ( Resource::BufferCtor(
			RCF_BUF_CONSTANT | RCF_ACE_CPU_WRITE , (uint32_t) blockSizes[i]
		) );
		sprintf( resourceName, "_constantCache%i", i );
		blockHandles[i] = DataBufferHandle::create( resourceName, &cbcs );

	}
}

ConstantCache::~ConstantCache() {
	for( int i = 0; i < CF_NUM_BLOCKS; ++i ) {
		blockHandles[i]->close();
	}
}

void ConstantCache::setObject(	const Math::Matrix4x4& prevWVPMatrix,
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

void ConstantCache::setWorldMatrix( const Math::Matrix4x4& worldMatrix ) {
	matrixCache[ CVN_WORLD ] = worldMatrix;
	cachedFlags &= ~(	
		BIT64(CVN_WORLD_INVERSE) | BIT64(CVN_WORLD_IT) |
		BIT64(CVN_WORLD_VIEW) | BIT64(CVN_WORLD_VIEW_INVERSE) | BIT64(CVN_WORLD_VIEW_IT) |
		BIT64(CVN_WORLD_VIEW_PROJ) | BIT64(CVN_WORLD_VIEW_PROJ_INVERSE) | BIT64(CVN_WORLD_VIEW_PROJ_IT)
	);
	cachedFlags |= BIT64(CVN_WORLD);
	gpuHasBlocks &= ~( BIT(CF_STD_OBJECT) );

}

void ConstantCache::setViewMatrix( const Math::Matrix4x4& viewMatrix ) {
	matrixCache[ CVN_VIEW ] = viewMatrix;
	// clearing out the whole block cache bits is a bit lazy but...
	cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_VIEW ] ] );
	cachedFlags &= ~(varBitsPerBlock [ CF_STD_OBJECT ] );

	cachedFlags |= BIT64(CVN_VIEW);
	gpuHasBlocks &= ~ ( BIT(varFreq[ CVN_VIEW ]) | BIT(CF_STD_OBJECT) );
}

void ConstantCache::setProjectionMatrix( const Math::Matrix4x4& projMatrix ) {
	matrixCache[ CVN_PROJ ] = projMatrix;
	// clearing out the whole block cache bits is a bit lazy but...
	cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_PROJ ] ] );
	cachedFlags &= ~(varBitsPerBlock [ CF_STD_OBJECT ] );

	cachedFlags |= BIT64(CVN_PROJ);
	gpuHasBlocks &= ~ ( BIT(varFreq[ CVN_PROJ ]) | BIT(CF_STD_OBJECT) );
}

void ConstantCache::setUserMatrix0( const Math::Matrix4x4& userMatrix ) {
	matrixCache[ CVN_USER_MATRIX_0 ] = userMatrix;
	cachedFlags |= BIT64(CVN_USER_MATRIX_0);
	gpuHasBlocks &= ~ BIT(varFreq[ CVN_USER_MATRIX_0 ]);
}

void ConstantCache::setUserMatrix1( const Math::Matrix4x4& userMatrix ) {
	matrixCache[ CVN_USER_MATRIX_1 ] = userMatrix;
	cachedFlags |= BIT64(CVN_USER_MATRIX_1);
	gpuHasBlocks &= ~BIT(varFreq[ CVN_USER_MATRIX_1 ]);
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
	case CVN_VIEW: 			return matrixCache[ type ];
	case CVN_VIEW_INVERSE: 	return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_VIEW) );
	case CVN_VIEW_IT: 		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_VIEW_INVERSE) );
	case CVN_PROJ:			return matrixCache[ type ];
	case CVN_PROJ_INVERSE:	return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_PROJ) );
	case CVN_PROJ_IT:		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_PROJ_INVERSE) );
	case CVN_VIEW_PROJ:		return matrixCache[ type ] = getMatrix(CVN_VIEW) * getMatrix(CVN_PROJ) ;
	case CVN_VIEW_PROJ_INVERSE:		return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_VIEW_PROJ) );
	case CVN_VIEW_PROJ_IT:	return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_VIEW_PROJ_INVERSE) );
	case CVN_WORLD:			return matrixCache[ type ];
	case CVN_WORLD_INVERSE:	return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD) );
	case CVN_WORLD_IT:		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_INVERSE) );
	case CVN_WORLD_VIEW:	return matrixCache[ type ] = getMatrix(CVN_WORLD) * getMatrix(CVN_VIEW);	
	case CVN_WORLD_VIEW_INVERSE:	return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD_VIEW) );
	case CVN_WORLD_VIEW_IT:	return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_VIEW_INVERSE) );
	case CVN_WORLD_VIEW_PROJ:		return matrixCache[ type ] = getMatrix(CVN_WORLD_VIEW) * getMatrix(CVN_PROJ);
	case CVN_WORLD_VIEW_PROJ_INVERSE:	return matrixCache[ type ] = InverseMatrix( getMatrix(CVN_WORLD_VIEW_PROJ) );
	case CVN_WORLD_VIEW_PROJ_IT:		return matrixCache[ type ] = TransposeMatrix( getMatrix(CVN_WORLD_VIEW_PROJ_INVERSE) );
	case CVN_PREV_WORLD_VIEW_PROJ:	return matrixCache[ type ];
	case CVN_USER_MATRIX_0:	return matrixCache[ type ];
	case CVN_USER_MATRIX_1:	return matrixCache[ type ];
	default: CORE_ASSERT( false && ("Invalid matrix type") ); return matrixCache[ 0 ];
	};
}

const Math::Vector4& ConstantCache::getVector( CONSTANT_VAR_NAME type ) const {
	return *(Math::Vector4*)&vectorCache[(type-CVN_NUM_MATRICES)*16];
}

void ConstantCache::getVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const {
	switch( type ) {
	case CVN_FRAMECOUNT:
	case CVN_TARGET_DIMS:
	case CVN_MATERIAL_INDEX:
	case CVN_LIGHT_COUNTS:
		memcpy( out4, &vectorCache[(type-CVN_NUM_MATRICES)*16], 16 );
		break;
	default:
		CORE_ASSERT( false && ("Invalid matrix type") );
	}
}

void ConstantCache::getRawVector( CONSTANT_VAR_NAME type, uint32_t* out4 ) const {
	memcpy( out4, &vectorCache[(type-CVN_NUM_MATRICES)*16], 16 );
}

void ConstantCache::getVector( CONSTANT_VAR_NAME type, float* out4 ) const {
	switch( type ) {
	case CVN_ZPLANES:
	case CVN_FOV:
		memcpy( out4, &vectorCache[(type-CVN_NUM_MATRICES)*16], 16 );
	default:	CORE_ASSERT( false && ("Invalid variable type") );
	}

}

void ConstantCache::setVector( CONSTANT_VAR_NAME type, const uint32_t* in4 ) {
	switch( type ) {
	case CVN_FRAMECOUNT:
	case CVN_TARGET_DIMS:
	case CVN_MATERIAL_INDEX:
		memcpy( &vectorCache[(type-CVN_NUM_MATRICES)*16], in4, 16 );
		break;
	default:	CORE_ASSERT( false && ("Variable isn't a uint32_t vector type") );
	}
}

void ConstantCache::setVector( CONSTANT_VAR_NAME type, const float* in4 ) {
	switch( type ) {
	case CVN_ZPLANES:
	case CVN_FOV:
		memcpy( &vectorCache[(type-CVN_NUM_MATRICES)*16], in4, 16 );
		break;
	default: 	CORE_ASSERT( false && ("Variable isn't a float vector type") );
	}
}

void ConstantCache::updateGPUBlock( Scene::RenderContext* context, CONSTANT_FREQ_BLOCKS block ) const {
	DataBufferPtr db = blockHandles[block]->acquire();
	int varsToSet = varsPerBlock[block];
	int count = 0;
	if( varsToSet > 0 ) {
		void* buffer = db->map( context, (RESOURCE_MAP_ACCESS)( RMA_WRITE | RMA_DISCARD ) );
		while( varsToSet > 0 ) {
			CORE_ASSERT( count < CVN_NUM_CONSTANTS );
			// TODO better than naive bit checking
			if( varBitsPerBlock[block] & BIT64(count) ) {
				if( count < CVN_NUM_MATRICES ) {
					Math::Matrix4x4 mat = Math::TransposeMatrix( getMatrix((CONSTANT_VAR_NAME)count) ); // compute if required
					memcpy( ((char*)buffer) + offsetInBlocks[ count ], 
							&mat, 
							sizeofInBlock[ count ] );
				} else {
					uint32_t uiArray[4];
					getRawVector( (CONSTANT_VAR_NAME)count, uiArray );
					memcpy( ((char*)buffer) + offsetInBlocks[ count ], 
							uiArray, 
							sizeofInBlock[ count ] );
				}
				--varsToSet;
			}
			count++;
		}
		db->unmap( context );
	}
	gpuHasBlocks |= BIT(block);
}

void ConstantCache::setCamera( const CameraPtr& camera ) {
	matrixCache[ CVN_VIEW ] = camera->getView();
	matrixCache[ CVN_PROJ ] = camera->getProjection();
	float zPlanes[4] = { camera->getZNear(), camera->getZFar(), 0, 0 };
	setVector( CVN_ZPLANES, zPlanes );
	float fov[4] = { camera->getXScale(), camera->getYScale(), 0, 0 };
	setVector( CVN_FOV, fov );

	// reset cached flags
	cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_VIEW ] ] );
	cachedFlags &= ~(varBitsPerBlock [ varFreq[ CVN_PROJ ] ] );
	cachedFlags &= ~(varBitsPerBlock [ CF_STD_OBJECT ] );

	cachedFlags |= BIT64(CVN_VIEW) | BIT64(CVN_PROJ) | BIT64(CVN_ZPLANES);
	gpuHasBlocks &= ~ ( BIT(varFreq[ CVN_VIEW ]) | BIT(varFreq[ CVN_PROJ ]) | BIT(CF_STD_OBJECT) );
}

void ConstantCache::updateGPU( Scene::RenderContext* context, const ProgramPtr prg ) {
	
	if( prg && (gpuHasBlocks & prg->getUsedBuffers()) == prg->getUsedBuffers() ) {
		// upto date, nothing to do
		return;
	}

	// what needs updating?
	uint32_t needsUpdating = (~gpuHasBlocks) & (prg ? prg->getUsedBuffers() : ~0);

	for( int i = 0; i < CF_NUM_BLOCKS; ++i ) {
		if( needsUpdating & BIT(i) ) {
			updateGPUBlock( context, (CONSTANT_FREQ_BLOCKS)i );
		}
	}
}
const DataBufferHandlePtr ConstantCache::getBlock( CONSTANT_FREQ_BLOCKS block ) const { 
	return blockHandles[ block ]; 
}

} // end Scene namespace
