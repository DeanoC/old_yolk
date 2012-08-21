//!-----------------------------------------------------
//!
//! @file debugpipeline.cpp
//! @brief A pipeline that represent a standard ol' fashioned
//! forward debug rendering pass.
//!
//!-----------------------------------------------------

#include "scene.h"
#include "core/resourceman.h"
#include "wobfile.h"
#include "texture.h"
#include "databuffer.h"
#include "vertexinput.h"
#include "rendercontext.h"
#include "wobfile.h"
#include "imagecomposer.h"
#include "debugpipeline.h"

namespace Scene {

DebugPipeline::DebugPipeline( ) {
	// NOTE programs are loaded off disk even when from the internal default programs (which are actually bound in the code)
	programHandle.reset( Scene::ProgramHandle::load( "basic" ) );

	Texture::CreationInfo ccs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET |RCF_PRG_READ,
			GTF_RGBA8,
			1024, 1024
		);
	static const std::string colTargetName = "DebugPipe_ColourTarget";
	colourTargetHandle.reset( TextureHandle::create( colTargetName.c_str(), &ccs ) );

	Texture::CreationInfo dcs = Texture::TextureCtor(
			RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_PRG_READ,
			GTF_DEPTH24_STENCIL8,
			1024, 1024
		);
	static const std::string depthTargetName = "DebugPipe_DepthTarget";
	depthTargetHandle.reset( TextureHandle::create( depthTargetName.c_str(), &dcs ) );

	rasterStateHandle.reset( RasteriserStateHandle::create( "Normal" ) );
}
	
DebugPipeline::~DebugPipeline() {
}

void DebugPipeline::bind( Scene::RenderContext* ctx ) {
	ctx->pushDebugMarker( getName() );

	auto colourTarget = colourTargetHandle.acquire();
	auto depthTarget = depthTargetHandle.acquire();

	ctx->clear( colourTarget, Core::RGBAColour(0,0,0,0) );
	ctx->clearDepthStencil( depthTarget, true, 1.0f, true, 0 );

	ctx->bindRenderTargets( colourTarget, depthTarget );

	auto program = programHandle.acquire();
	ctx->getConstantCache().updateGPU( ctx, program );
	ctx->bind( program );

	auto rasterState = rasterStateHandle.acquire();
	ctx->bind( rasterState );

//	glEnable( GL_DEPTH_TEST );
//	glDepthMask(GL_TRUE);
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_FRONT);
}

void DebugPipeline::unbind( Scene::RenderContext* ctx) {
	ctx->popDebugMarker();
}

void DebugPipeline::conditionWob( Scene::Wob* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	auto program = programHandle.acquire();

	DebugPipelineDataStore* pds =  CORE_NEW DebugPipelineDataStore();
	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY DebugPipelineDataStore::PerMaterial[ pds->numMaterials ] );
	wob->pipelineDataStores[pipelineIndex] = std::unique_ptr<DebugPipelineDataStore>( pds );

	const std::string name( header->pName.p );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		DebugPipelineDataStore::PerMaterial* mds = &pds->materials[i];
		mat->backEndData.p = (void*)mds;

		mds->name = name + std::string( mat->pName.p );
		const int indexSize = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? sizeof(uint32_t) : sizeof(uint16_t);
		mds->numIndices = mat->uiNumIndices;
		mds->indexSize = indexSize;

		// index buffer
		DataBuffer::CreationInfo indbcs = Resource::BufferCtor(
			RCF_ACE_IMMUTABLE | RCF_BUF_INDEX, mat->uiNumIndices * indexSize, mat->pIndexData.p
		);
		mds->indexBuffer = DataBufferHandle::create( (mds->name + "_ib").c_str(), &indbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		mds->vacs.elementCount = mat->numVertexElements;
		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.elements[i].type = (VIN_TYPE) mat->pElements.p[i].uiType; // identical at the moment
			mds->vacs.elements[i].usage = (VIN_ELEMENT) mat->pElements.p[i].uiUsage; // identical at the moment
		}
		const size_t vertexSize = VertexInput::getVertexSize( mds->vacs.elementCount, mds->vacs.elements );
		DataBuffer::CreationInfo vdbcs = Resource::BufferCtor(
			RCF_ACE_IMMUTABLE | RCF_BUF_VERTEX, (uint32_t)(mat->uiNumVertices *  vertexSize), mat->pVertexData.p
		);
		DataBufferHandlePtr vertexBuffer = DataBufferHandle::create( (mds->name + "_vb").c_str(), &vdbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.data[i].buffer = vertexBuffer;
			mds->vacs.data[i].offset = VI_AUTO_OFFSET;
			mds->vacs.data[i].stream = 0;
			mds->vacs.data[i].stride = VI_AUTO_STRIDE;
		}

		mds->vaoHandle = VertexInputHandle::create( 
				(mds->name + VertexInput::genEleString(mds->vacs.elementCount, mds->vacs.elements)).c_str(), &mds->vacs );
		mds->vaoHandle->acquire()->validate( program );
	}
}

void DebugPipelineDataStore::render( Scene::RenderContext* rc ) {

	RenderContext* ctx = (RenderContext*) rc;
	ctx->getConstantCache().updateGPUBlock( rc, Scene::CF_STD_OBJECT );

	for( int i = 0;i < numMaterials; ++i ) {
		const PerMaterial* mds = &materials[i];

		auto vao = mds->vaoHandle->tryAcquire();
		if( !vao ) { /* LOG(INFO) << "vao not ready\n"; */ return; }
		auto ib = mds->indexBuffer->tryAcquire();
		if( !ib ) { /* LOG(INFO) << "ib not ready\n"; */ return; }
		ctx->bind( vao );
		ctx->bindIndexBuffer( ib );
		ctx->drawIndexed( PT_TRIANGLE_LIST, mds->numIndices );
//		glBindVertexArray( vao->getName() );
//		GL_CHECK
//		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->getName() );
//		GL_CHECK
//		glDrawElements( GL_TRIANGLES, mds->numIndices, mds->indexType, 0 );
//		GL_CHECK
	}

}

DebugPipelineDataStore::~DebugPipelineDataStore() {
	for( int i = 0;i < numMaterials; ++i ) {
		DebugPipelineDataStore::PerMaterial* mds = &materials[i];

		if( mds->indexBuffer )
			mds->indexBuffer->close();

		if( mds->vacs.data[0].buffer )
			mds->vacs.data[0].buffer->close();

		if(mds->vaoHandle)
			mds->vaoHandle->close();
	}
}

}
