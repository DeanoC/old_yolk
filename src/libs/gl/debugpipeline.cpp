//!-----------------------------------------------------
//!
//! @file debugpipeline.cpp
//! @brief A pipeline that represent a standard ol' fashioned
//! forward debug rendering pass.
//!
//!-----------------------------------------------------

#include "ogl.h"
#include "core/resourceman.h"
#include "scene/wobfile.h"
#include "gfx.h"
#include "texture.h"
#include "databuffer.h"
#include "vao.h"
#include "rendercontext.h"
#include "wobbackend.h"
#include "scene/imagecomposer.h"
#include "debugpipeline.h"

namespace Gl {

DebugPipeline::DebugPipeline( size_t index ) :
	Scene::Pipeline( index )
{
	programHandle.reset( Scene::ProgramHandle::create( "basic" ) );
}
	
DebugPipeline::~DebugPipeline() {
}

void DebugPipeline::bind( Scene::RenderContext* rc ) {

	context = (Gl::RenderContext*) rc;
	context->pushDebugMarker( getName() );

	// render direct to back buffer and back depth buffer
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	auto program = programHandle.acquire();
	context->bindProgram( program );
	context->getConstantCache().updateGPU( program );
	context->bindConstants();

	glEnable( GL_DEPTH_TEST );
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
}

void DebugPipeline::unbind() {
	context->popDebugMarker();
	context = nullptr;
}

void DebugPipeline::conditionWob( const char* cname, Scene::WobResource* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	DebugPipelineDataStore* pds =  CORE_NEW DebugPipelineDataStore();
	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY DebugPipelineDataStore::PerMaterial[pds->numMaterials] );
	Gl::WobBackEnd* backEnd = (Gl::WobBackEnd*) wob->backEnd.get();
	backEnd->pipelineDataStores[pipelineIndex] = std::unique_ptr<DebugPipelineDataStore>(pds);

	const std::string name( cname );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		DebugPipelineDataStore::PerMaterial* mds = &pds->materials[i];

		mds->name = name + std::string( mat->pName.p );
		const int indexSize = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? sizeof(uint32_t) : sizeof(uint16_t);

		// index buffer
		DataBuffer::CreationStruct indbcs = {
			DBCF_IMMUTABLE, DBT_INDEX, mat->uiNumIndices * indexSize, mat->pIndexData.p
		};

		mds->indexBuffer = DataBufferHandle::create( (mds->name + "_ib").c_str(), &indbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		mds->numIndices = mat->uiNumIndices;
		mds->indexType = (indexSize == sizeof(uint32_t)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		mds->vacs.elementCount = mat->numVertexElements;
		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.elements[i].type = (VIN_TYPE) mat->pElements.p[i].uiType; // identical at the moment
			mds->vacs.elements[i].usage = (VIN_ELEMENT) mat->pElements.p[i].uiUsage; // identical at the moment
		}
		size_t vertexSize = Vao::getVertexSize( mds->vacs.elementCount, mds->vacs.elements );

		DataBuffer::CreationStruct vdbcs = {
			DBCF_IMMUTABLE, DBT_VERTEX, mat->uiNumVertices *  vertexSize, mat->pVertexData.p
		};
		DataBufferHandlePtr vertexBuffer = DataBufferHandle::create( (mds->name + "_vb").c_str(), &vdbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.data[i].buffer = vertexBuffer;
			mds->vacs.data[i].offset = VI_AUTO_OFFSET;
			mds->vacs.data[i].stream = 0;
			mds->vacs.data[i].stride = VI_AUTO_STRIDE;
		}
		mds->vaoHandle = 0; // defer till renderomg
	}
}
void DebugPipeline::display( Scene::RenderContext* rc, int backWidth, int backHeight ) {
	// nothing to do as already in backbuffer
}

void DebugPipeline::merge( Scene::RenderContext* rc ) {
	// can't be merged into another pipeline, as goes direct to backbuffer
	CORE_ASSERT( false );
}

void DebugPipelineDataStore::render( Scene::RenderContext* rc ) {

	RenderContext* context = (RenderContext*) rc;
	context->getConstantCache().updateGPUObjectOnly();

	for( int i = 0;i < numMaterials; ++i ) {
		const PerMaterial* mds = &materials[i];
		if( mds->vaoHandle == 0 ) {
			PerMaterial* mdsw = const_cast<PerMaterial*>(mds);
			// defer creation, as vao have to be created sync which required acquire and could force other resource to be acquired
			mdsw->vaoHandle = Scene::VertexInputHandle::create( (mds->name + Vao::genEleString(mds->vacs.elementCount, mds->vacs.elements)).c_str(), &mds->vacs );
		}

		auto vao = std::static_pointer_cast<Gl::Vao>( mds->vaoHandle->tryAcquire() );
		if( !vao ) {
//			LOG(INFO) << "vao not ready\n";
			return;
		}
		auto ib = std::static_pointer_cast<Gl::DataBuffer>( mds->indexBuffer->tryAcquire() );
		if( !ib ) { /* LOG(INFO) << "ib not ready\n"; */ return; }

		glBindVertexArray( vao->getName() );
		GL_CHECK
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->getName() );
		GL_CHECK
		glDrawElements( GL_TRIANGLES, mds->numIndices, mds->indexType, 0 );
		GL_CHECK
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
