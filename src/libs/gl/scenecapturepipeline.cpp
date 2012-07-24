//!-----------------------------------------------------
//!
//! @file scenecapturepipeline.cpp
//! @brief A pipeline for my experimental scene capture
//! renderer.
//!
//!-----------------------------------------------------

#include "gl.h"
#include "core/resources.h"
#include "scene/wobfile.h"
#include "gfx.h"
#include "shaderman.h"
#include "texture.h"
#include "databuffer.h"
#include "vao.h"
#include "rendercontext.h"
#include "gpu_constants.h"
#include "wobbackend.h"

// compute renderer needs CL
#include "cl/cl.h"
#include "cl/platform.h"
#include "cl/kernel.h"
#include "cl/image.h"
#include "cl/programman.h"
#include "cl/buffer.h"


#include "scenecapturepipeline.h"

// outside namespace!
DECLARE_SHARED_WITH_CL( scenecapturestruct );
DECLARE_FRAGMENT( vs_scenecapturecapture );
DECLARE_FRAGMENT( gs_scenecapturecapture );
DECLARE_FRAGMENT( fs_scenecapturecapture );

DECLARE_PROGRAM( scenecapture );
DECLARE_CL_LIBRARY( scenecapture_debug );
DECLARE_CL_LIBRARY( scenecapture_bitpackchain );
DECLARE_CL_LIBRARY( scenecapture_testlighting );

namespace Gl {

SceneCapturePipeline::SceneCapturePipeline( size_t index ) :
	pipelineIndex( index )
{
	REGISTER_SHARED_WITH_CL( scenecapturestruct );
	REGISTER_FRAGMENT( vs_scenecapturecapture );
	REGISTER_FRAGMENT( gs_scenecapturecapture );
	REGISTER_FRAGMENT( fs_scenecapturecapture );

	REGISTER_PROGRAM( scenecapture );
	REGISTER_CL_LIBRARY( scenecapture_debug );
	REGISTER_CL_LIBRARY( scenecapture_bitpackchain );
	REGISTER_CL_LIBRARY( scenecapture_testlighting );

	contextCl = Cl::Platform::get()->getPrimaryContext().get();

	// colour + counter + z min max GL render target + CL image objects
	Texture::CreationStruct sccrt = {
		TCF_2D | TCF_RENDER_TARGET | TCF_GPU_WRITE_ONLY,
		GL_SRGB8_ALPHA8,
		Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight(), 1, 1
	};
	colourRtHandle.reset( TextureHandle::create( "_scenecapturepipe_colrt", &sccrt ) );
	sccrt.format = TF_RGBA8888; // no SRGB
	counterRtHandle.reset( TextureHandle::create( "_scenecapturepipe_counterrt", &sccrt ) );
	sccrt.format = GL_RGBA32F; // note we pretend its float so the blender works BUT actual 24 bit integer
	minMaxRtHandle.reset( TextureHandle::create( "_scenecapturepipe_minmaxrt", &sccrt ) );
	Cl::Image::CreationStruct clihcs = {
		contextCl,
		(Cl::IMAGE_CREATION_FLAGS) (Cl::ICF_FROM_GL | Cl::ICF_KERNEL_WRITE | Cl::ICF_KERNEL_READ),
		(Cl::IMAGE_FORMAT)0, 0, 0, (void*) colourRtHandle.get()
	};
	colourClRtHandle.reset( Cl::ImageHandle::create( "_scenecapturepipe_cl_colour_image", &clihcs ) );
	clihcs.data = (void*) counterRtHandle.get();
	counterClRtHandle.reset( Cl::ImageHandle::create( "_scenecapturepipe_cl_counter_image", &clihcs ) );
	clihcs.data = (void*) minMaxRtHandle.get();
	minMaxClRtHandle.reset( Cl::ImageHandle::create( "_scenecapturepipe_cl_minMax_image", &clihcs ) );

	// fragment + head GL buffer, GL texture, CL Buffer
	static const int MAX_FRAGMENTS = 10 * 1024 * 1024 * (128/8); // each fragment is 128 bits (4xuint32_t)
	DataBuffer::CreationStruct fbcs = {
		DBCF_NONE, DBT_TEXTURE, MAX_FRAGMENTS, nullptr
	};
	fragmentsBufferHandle.reset( DataBufferHandle::create( "_scenecapturepipe_fragmentsbuffer", &fbcs ) );
	fbcs.size = Gfx::get()->getScreenWidth() * Gfx::get()->getScreenHeight() * sizeof(uint32_t);
	headsBufferHandle.reset( DataBufferHandle::create( "_scenecapturepipe_headsbuffer", &fbcs ) );
	auto fragmentsBuffer = fragmentsBufferHandle.acquire();
	auto headsBuffer = headsBufferHandle.acquire();
	Texture::CreationStruct fragcs = {
		TCF_BUFFER, GL_RGBA32UI, 1, 1, 1, 1, fragmentsBuffer->getName()
	};
	fragmentsTexHandle.reset( TextureHandle::create( "_scenecapturepipe_fragments_tex", &fragcs ) );
	fragcs.bufferName = headsBuffer->getName();
	fragcs.format = GL_R32UI,
	headsTexHandle.reset( TextureHandle::create( "_scenecapturepipe_heads_tex", &fragcs ) );
	Cl::Buffer::CreationStruct clfbcs = {
		contextCl,
		(Cl::BUFFER_CREATION_FLAGS) (Cl::BCF_FROM_GL | Cl::BCF_KERNEL_WRITE | Cl::BCF_KERNEL_READ ),
		0, (void*) fragmentsBufferHandle.get()
	};
	fragmentsClBufferHandle.reset( Cl::BufferHandle::create( "_scenecapturepipe_cl_fragments", &clfbcs ) );
	clfbcs.data = (void*) headsBufferHandle.get();
	headsClBufferHandle.reset( Cl::BufferHandle::create( "_scenecapturepipe_cl_heads", &clfbcs ) );


	// GL atomic counter buffer + zero'd clone for first reset
	// current fragmentAtomic and primitiveAtomic are in here
	DataBuffer::CreationStruct atbcs = {
		(DATA_BUFFER_CREATION_FLAGS) DBCF_NONE, DBT_ATOMIC_COUNTERS, sizeof(uint32_t) * 2, nullptr
	};
	atomicCounterHandle.reset( DataBufferHandle::create( "_scenecapturepipe_atomicounters", &atbcs ) );
	atbcs.flags = DBCF_IMMUTABLE;
	atbcs.data = CORE_STACK_ALLOC( atbcs.size );
	memset( atbcs.data, 0, atbcs.size );
	atomicCounterClearerHandle.reset( DataBufferHandle::create( "_scenecapturepipe_atomicounters_clearer", &atbcs ) );

	// CL bit packed to put compute z occupancy in
	Cl::Buffer::CreationStruct zobcs = {
		contextCl, (Cl::BUFFER_CREATION_FLAGS) (Cl::BCF_KERNEL_WRITE | Cl::BCF_KERNEL_READ), 
		Gfx::get()->getScreenWidth() * Gfx::get()->getScreenHeight() * sizeof(uint64_t)
	};
	zOccupiedBitsHandle.reset( Cl::BufferHandle::create( "_scenecapturepipe_zoccupy_buffer", &zobcs ) );
	static const int MAX_GPU_LIGHTS = 32;
	Cl::Buffer::CreationStruct libcs = {
		contextCl, (Cl::BUFFER_CREATION_FLAGS) (Cl::BCF_KERNEL_WRITE), 
		MAX_GPU_LIGHTS * sizeof( GPUConstants::Light )
	};
	lightsHandle.reset( Cl::BufferHandle::create( "_scenecapturepipe_lights_buffer", &libcs ) );
	lightsMem.reset( CORE_NEW_ARRAY GPUConstants::Light[ MAX_GPU_LIGHTS ] );

	static const int MAX_POST_TRANSFORM_VERTEX_BYTES = 1024 * 1024 * (256/8);
	DataBuffer::CreationStruct xbbcs = {
		DBCF_NONE, DBT_VERTEX, MAX_POST_TRANSFORM_VERTEX_BYTES, nullptr
	};
	primitivesBufferHandle.reset( DataBufferHandle::create( "_scenecapturepipe_geometry_transformed", &xbbcs ) );
	Cl::Buffer::CreationStruct xbcbcs = {
		contextCl,
		(Cl::BUFFER_CREATION_FLAGS) (Cl::BCF_FROM_GL | Cl::BCF_KERNEL_READ ),
		0, (void*) primitivesBufferHandle.get()
	};
	primitivesClBufferHandle.reset( Cl::BufferHandle::create( "_scenecapturepipe_cl_geometry_transformed", &xbcbcs ) );
	auto primitivesBuffer = primitivesBufferHandle.acquire();
	Texture::CreationStruct prfragcs = {
		TCF_BUFFER, GL_RGBA32UI, 1, 1, 1, 1, primitivesBuffer->getName()
	};
	primitivesTexHandle.reset( TextureHandle::create( "_scenecapturepipe_geometry_tex", &prfragcs ) );

	// Cl kernels for this pipeline
	Cl::Kernel::CreationStruct dkcs = {
		contextCl,
		(Cl::KERNEL_CREATION_FLAGS) 0,
		(Cl::DEVICE_EXTENSIONS_FLAGS) 0
	};
	countFragmentsHandle.reset( Cl::KernelHandle::create("scenecapture_debug|countFragments", &dkcs) );
	countFragmentChainsHandle.reset( Cl::KernelHandle::create("scenecapture_debug|countFragmentChain", &dkcs) );
	debugZOccupiedBitsHandle.reset( Cl::KernelHandle::create("scenecapture_debug|debugZOccupiedBits", &dkcs) );
	bitPackFragmentChainsHandle.reset( Cl::KernelHandle::create("scenecapture_bitpackchain|bitpackFragmentChain", &dkcs) );
	headCleanerHandle.reset( Cl::KernelHandle::create("scenecapture_bitpackchain|headCleaner", &dkcs) );

	// debug kernels args
	auto countFragments = countFragmentsHandle.acquire();
	countFragments->setArg( 0, counterClRtHandle );
	countFragments->setArg( 1, colourClRtHandle );
	auto countFragmentChains = countFragmentChainsHandle.acquire();
	countFragmentChains->setArg( 0, headsClBufferHandle );
	countFragmentChains->setArg( 1, fragmentsClBufferHandle );
	countFragmentChains->setArg( 2, colourClRtHandle );
	auto debugZOccupiedBits = debugZOccupiedBitsHandle.acquire();
	debugZOccupiedBits->setArg( 0, zOccupiedBitsHandle );
	debugZOccupiedBits->setArg( 1, colourClRtHandle );


	// no vis
/*	
	lightTestHandle.reset( Cl::KernelHandle::Create("scenecapture_testlighting|testlighting_novis_0bounce", &dkcs) );
	auto lightTest = lightTestHandle.Acquire();
	lightTest->setArg( 0, headsClBufferHandle );
	lightTest->setArg( 1, fragmentsClBufferHandle );
	lightTest->setArg( 2, primitivesClBufferHandle );
	lightTest->setArg( 3, lightsHandle );
	lightTest->setArg( 4, colourClRtHandle );
*/
	// with vis
	lightTestHandle.reset( Cl::KernelHandle::create("scenecapture_testlighting|testlighting_0bounce", &dkcs) );
	auto lightTest = lightTestHandle.acquire();
	lightTest->setArg( 0, headsClBufferHandle );
	lightTest->setArg( 1, fragmentsClBufferHandle );
	lightTest->setArg( 2, primitivesClBufferHandle );
	lightTest->setArg( 3, lightsHandle );
	lightTest->setArg( 4, zOccupiedBitsHandle );
	lightTest->setArg( 5, minMaxClRtHandle );	
	// set per frame
	lightTest->setArg( 7, colourClRtHandle );

	// working kernels args
	// take head + fragment chains, producing closest fragment at head and bit packed occupancy
	auto bitPackFragmentChains = bitPackFragmentChainsHandle.acquire();
	bitPackFragmentChains->setArg( 0, headsClBufferHandle );
	bitPackFragmentChains->setArg( 1, fragmentsClBufferHandle );
	bitPackFragmentChains->setArg( 2, zOccupiedBitsHandle );
	bitPackFragmentChains->setArg( 3, minMaxClRtHandle );
	auto headCleaner = headCleanerHandle.acquire();
	headCleaner->setArg( 0, headsClBufferHandle );

	// GL programs
	sceneCaptureProgramHandle.reset( ProgramHandle::create( "scenecapture" ) );

	// test lights
	lightsMem[0].pos = Math::Vector4(0,25,10,0);
	lightsMem[0].wo = Math::Normalise( Math::Vector4(-4, 35, 20, 0) - lightsMem[0].pos );
	lightsMem[0].flux = Math::Vector4(1 * 2500, 1 * 2500, 1 * 2500, 0);
	lightsMem[0].barnDoorDot = -0.85f;
	lightsMem[1].pos = Math::Vector4(30, 35.f, -10, 0);
	lightsMem[1].wo = Math::Normalise( Math::Vector4(0,20,10,0) - lightsMem[1].pos );
	lightsMem[1].flux = Math::Vector4(0.2f * 2700, 0.2f * 2700, 0.7f *2470, 0);
	lightsMem[1].barnDoorDot = -0.9f;

	// final light + 1 has |wo| == 0 as end marker
	lightsMem[2].wo = Math::Vector4(0,0,0,0);

}


SceneCapturePipeline::~SceneCapturePipeline() {
}

void SceneCapturePipeline::bind( Scene::RenderContext* rc, bool clear ) {
	RenderContext* context = (RenderContext*) rc;
	context->pushDebugMarker( "SceneCapturePipeline" );

	Cl::CmdQueuePtr q = contextCl->getCmdQueue(0);
	auto lights = lightsHandle.acquire();
	q->write( lightsMem.get(), lights, &lightsUploadedEvent );

	const uint32_t frameCount[4] = { Gfx::get()->getFrameCount(), 0, 0, 0 };
	context->getConstantCache().setUIVector( CVN_FRAMECOUNT, frameCount );
	const uint32_t targetDims[4] = { Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight(), 0, 0 };
	context->getConstantCache().setUIVector( CVN_TARGET_DIMS, targetDims );

	ProgramPtr program = sceneCaptureProgramHandle.acquire();

	auto fragments = fragmentsTexHandle.acquire();
	auto heads = headsTexHandle.acquire();
	auto primitives = primitivesTexHandle.acquire();

	// clear atomics via a gpu -> gpu copy (should be faster way, as buffer is small)
	auto atomics = atomicCounterHandle.acquire();
	auto atomicsClearer = atomicCounterClearerHandle.acquire();
	glBindBuffer( GL_COPY_READ_BUFFER, atomicsClearer->getName() );
	glBindBuffer( GL_COPY_WRITE_BUFFER, atomics->getName() );
	glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, atomicsClearer->getSize() );

	// clear heads buffer, TODO faster to use GL?
	auto headsCl = headsClBufferHandle.acquire();
	q->acquireGL( headsCl->getName() );
	auto headCleaner = headCleanerHandle.acquire();
	size_t glbSize[2] = { Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight() };
	size_t lclSize[2] = { 16, 16 };
	q->execute( headCleaner, Cl::WorkRange<2>( glbSize, lclSize ) );
	q->releaseGL( headsCl->getName() );

	// clear the render targets 
	auto counterRt = counterRtHandle.acquire();
	auto minMaxRt = minMaxRtHandle.acquire();
	TexturePtr	renderTargets[] = { counterRt, minMaxRt };
	context->useAsRenderTargets( 2, renderTargets );
	if( clear ) {
		float colClr0[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearBufferfv( GL_COLOR, 0, colClr0 );
		uint32_t colClr1[4] = { 0xFFFFFFFF, 0, 0, 0 };
		glClearBufferuiv( GL_COLOR, 1, colClr1 );
	}	
	
	context->bindWholeProgram( program );
	context->getConstantCache().updateGPU( program );
	context->getConstantCache().bind();

	// set up blending, add first target, min max red and alpha 2nd target (depth)
	glEnable( GL_BLEND );
	glEnablei( GL_BLEND, 0 );
	glEnablei( GL_BLEND, 1 );
	GL_CHECK
	// target 0: src RGB + dst RGB, src Alpha + dst Alpha
	glBlendEquationSeparatei( 0, GL_FUNC_ADD, GL_FUNC_ADD );
	glBlendFuncSeparatei( 0, GL_ONE, GL_ONE, GL_ONE, GL_ONE );
	// target 1: Min( src R, dst R ), Max( src Alpha, dst Alpha )
	glBlendEquationSeparatei( 1, GL_MIN, GL_MAX );
	glBlendFuncSeparatei( 1, GL_ONE, GL_ONE, GL_ONE, GL_ONE );
	GL_CHECK
	glBindImageTexture( 0, fragments->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32UI );
	GL_CHECK
	glBindImageTexture( 1, heads->getName(), 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI );
	GL_CHECK
	glBindImageTexture( 2, primitives->getName(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32UI );
	GL_CHECK
	glBindBufferBase( GL_ATOMIC_COUNTER_BUFFER, 0, atomics->getName() );
	GL_CHECK

}

void SceneCapturePipelineDataStore::render( Scene::RenderContext* rc ) {

	RenderContext* context = (RenderContext*) rc;
	context->getConstantCache().updateGPUObjectOnly();

	VaoPtr vao = vaoHandle->acquire();
	DataBufferPtr ib = vacs.indexBuffer->acquire();

	glBindVertexArray( vao->getName() );
	GL_CHECK	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->getName() );
	GL_CHECK	
	glDrawElements( GL_TRIANGLES, numIndices, indexType, 0 );
	GL_CHECK	

}


void SceneCapturePipeline::unbind( Scene::RenderContext* rc ) {

	RenderContext* context = (RenderContext*) rc;
	glBindVertexArray( 0 );
	GL_CHECK	
	glBindBuffer( GL_ATOMIC_COUNTER_BUFFER, 0 );
	GL_CHECK	
	glBindImageTexture( 0, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32UI );
	glBindImageTexture( 1, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI );
	glBindImageTexture( 2, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI );
	GL_CHECK	

	glDisable( GL_BLEND );
	GL_CHECK	

	context->unbindWholeProgam();
	context->getConstantCache().unbind();

	auto ccPerViewBufferHandle = context->getConstantCache().getClBlock( Gl::CF_PER_VIEWS ) ;

	auto heads = headsClBufferHandle.acquire();
	auto fragments = fragmentsClBufferHandle.acquire();
	auto colourRt = colourClRtHandle.acquire();
	auto counterRt = counterClRtHandle.acquire();
	auto minMaxRt = minMaxClRtHandle.acquire();
	auto transformFeedback = primitivesClBufferHandle.acquire();
	auto ccPerViewBuffer = ccPerViewBufferHandle->acquire();
//	auto viskernel = countFragmentsHandle.acquire();
//	auto viskernel = countFragmentChainsHandle.acquire();
//	auto viskernel = debugZOccupiedBitsHandle.acquire();

	auto viskernel = lightTestHandle.acquire();
	// for visability
	viskernel->setArg( 6, ccPerViewBufferHandle );

	auto kernel = bitPackFragmentChainsHandle.acquire();

	Cl::CmdQueuePtr q = contextCl->getCmdQueue(0);
	q->acquireGL( heads->getName() );
	q->acquireGL( fragments->getName() );
	q->acquireGL( colourRt->getName() );
	q->acquireGL( counterRt->getName() );
	q->acquireGL( minMaxRt->getName() );
	q->acquireGL( transformFeedback->getName() );
	q->acquireGL( ccPerViewBuffer->getName() );

	size_t glbSize[2] = { Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight() };
	size_t lclSize[2] = { 16, 16 };
	q->execute( kernel, Cl::WorkRange<2>( glbSize, lclSize ) );
	q->waitForEvents( 1, &lightsUploadedEvent );
	q->execute( viskernel, Cl::WorkRange<2>( glbSize, lclSize ) );

	q->releaseGL( ccPerViewBuffer->getName() );
	q->releaseGL( transformFeedback->getName() );
	q->releaseGL( minMaxRt->getName() );
	q->releaseGL( counterRt->getName() );
	q->releaseGL( colourRt->getName() );
	q->releaseGL( fragments->getName() );
	q->releaseGL( heads->getName() );

	clReleaseEvent( lightsUploadedEvent );

	auto colourGlRt = colourRtHandle.acquire();
	context->useAsRenderTarget( colourGlRt );

	context->popDebugMarker();
}

void SceneCapturePipeline::conditionWob( const char* cname, Scene::WobResource* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();
	const std::string name( cname );

	size_t numVertices = 0;
	size_t numIndices = 0;
	// pack all materials into a single object vbo+ibo+vao
	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];
		numVertices += mat->uiNumVertices;
		numIndices += mat->uiNumIndices;
	}
	std::vector<SceneCapturePipelineDataStore::PNVertex> tmpVertices;
	tmpVertices.resize( numVertices );

	GLenum indexType = GL_UNSIGNED_SHORT;
	int indexSize = sizeof(uint16_t);
	if( numVertices > 0xFFFE ) {
		indexType = GL_UNSIGNED_INT;
		indexSize = sizeof(uint32_t);
	}
	std::vector<uint8_t> tmpIndices;
	tmpIndices.resize( numIndices * ((indexType == GL_UNSIGNED_INT) ? 4 : 2) );

	size_t curVert = 0;
	unsigned char* outIndexPtr = &tmpIndices[0];

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		char* inIndexPtr = (char*) mat->pIndexData.p;
		const int inIndexSize = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? sizeof(uint32_t) : sizeof(uint16_t);
		for( size_t j = 0; j < mat->uiNumIndices; ++j ) {

			uint32_t index = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? *((uint32_t*)inIndexPtr) : *((uint16_t*)inIndexPtr);
			index += curVert;
			if(indexType == GL_UNSIGNED_INT) {
				*((uint32_t*)outIndexPtr) = index;
			} else {
				CORE_ASSERT( index < 0xFFFF );
				*((uint16_t*)outIndexPtr) = (uint16_t)index;
			}
			inIndexPtr += inIndexSize;			
			outIndexPtr += indexSize;
		}

		// TODO Wob Cracker
		const size_t inVertexSize = Vao::getVertexSize( mat->numVertexElements, (VaoElement*) mat->pElements.p );

		char* inVertexPtr = (char*) mat->pVertexData.p;
		for( size_t j = 0; j < mat->uiNumVertices; ++j ) {
			memcpy( &tmpVertices[curVert].pos, inVertexPtr + 0, sizeof(float)*3 );
			memcpy( &tmpVertices[curVert].norm, inVertexPtr + sizeof(float)*3, sizeof(float)*3 );
			inVertexPtr += inVertexSize;
			curVert++;
		}

	}

	// vertex buffer
	DataBuffer::CreationStruct vdbcs = {
		DBCF_IMMUTABLE, DBT_VERTEX, numVertices *  sizeof(SceneCapturePipelineDataStore::PNVertex),
		&tmpVertices[0]
	};
	DataBufferHandlePtr vertexBuffer = DataBufferHandle::create( (name + "_scp_vb").c_str(), &vdbcs );
	// index buffer
	DataBuffer::CreationStruct indbcs = {
		DBCF_IMMUTABLE, DBT_INDEX, numIndices * indexSize, &tmpIndices[0]
	};
	DataBufferHandlePtr indexBuffer = DataBufferHandle::create( (name + "_scp_ib").c_str(), &indbcs );

	Vao::CreationStruct vocs = {
		2,
		indexBuffer,
		{
			{ VE_POSITION, VT_FLOAT3 },
			{ VE_NORMAL, VT_FLOAT3 },
		},
		{
			vertexBuffer, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0,
			vertexBuffer, Vao::AUTO_OFFSET, Vao::AUTO_STRIDE, 0,
		}
	};

	const std::string vaoName = (name + "_scp_") + Vao::genEleString(vocs.elementCount, vocs.elements );
	VaoHandlePtr vaoHandle = VaoHandle::create( vaoName.c_str(), &vocs );

	SceneCapturePipelineDataStore* pds =  CORE_NEW SceneCapturePipelineDataStore();
	pds->indexType = indexType;
	pds->numIndices = numIndices;
	pds->vaoHandle = vaoHandle;
	memcpy( &pds->vacs, &vocs, sizeof( Vao::CreationStruct ) );
	WobBackEnd* backEnd = (WobBackEnd*)wob->backEnd.get();
	backEnd->pipelineDataStores[pipelineIndex] = std::unique_ptr<SceneCapturePipelineDataStore>(pds);

}

void SceneCapturePipeline::display( Scene::RenderContext* rc, int backWidth, int backHeight ) {
	RenderContext* context = (RenderContext*) rc;

	TexturePtr colourRt = colourRtHandle.acquire();
	context->useAsRenderTargets( 1, { &colourRt } );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	GL_CHECK
	glBlitFramebuffer(	0, 0, colourRt->getWidth(), colourRt->getHeight(),
							0, 0, backWidth, backHeight,
							GL_COLOR_BUFFER_BIT, GL_NEAREST);
	GL_CHECK
}

SceneCapturePipelineDataStore::~SceneCapturePipelineDataStore() {
	if( vacs.indexBuffer )
		vacs.indexBuffer->close();

	if( vacs.data[0].buffer )
		vacs.data[0].buffer->close();

	if(vaoHandle)
		vaoHandle->close();
}

}
