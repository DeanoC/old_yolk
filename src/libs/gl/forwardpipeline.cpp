//!-----------------------------------------------------
//!
//! @file forwardpipeline.cpp
//! @brief A pipeline that represent a standard ol' fashioned
//! forward rendering pass.
//!
//!-----------------------------------------------------

#include "gl.h"
#include "core/resourceman.h"
#include "scene/wobfile.h"
#include "gfx.h"
#include "texture.h"
#include "databuffer.h"
#include "vao.h"
#include "rendercontext.h"
#include "wobbackend.h"
#include "forwardpipeline.h"

namespace Gl {

ForwardPipeline::ForwardPipeline( size_t index ) :
	pipelineIndex( index )
{

	Texture::CreationStruct fpcrt = {
		TCF_2D | TCF_RENDER_TARGET,
		TF_RGBA8888, 1,
		Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight(),
	};
	colourRtHandle.reset( TextureHandle::create( "_forwardpipe_colrt", &fpcrt ) );

	Texture::CreationStruct fpdrt = {
		TCF_2D | TCF_RENDER_TARGET,
		GL_DEPTH24_STENCIL8, 1,
		Gfx::get()->getScreenWidth(), Gfx::get()->getScreenHeight(),
	};
	depthRtHandle.reset( TextureHandle::create( "_forwardpipe_depthrt", &fpdrt ) );

	programHandle.reset( ProgramHandle::create( "basic" ) );
}
	
ForwardPipeline::~ForwardPipeline() {
}

void ForwardPipeline::bind( Scene::RenderContext* rc, bool clear ) {

	Gl::RenderContext* context = (Gl::RenderContext*) rc;
	context->pushDebugMarker( "ForwardPipeline" );

	TexturePtr colourRt = colourRtHandle.acquire();
	TexturePtr depthRt = depthRtHandle.acquire();
	ProgramPtr program = programHandle.acquire();

	context->useAsRenderTargets( colourRt, depthRt );
	glViewport( 0, 0, colourRt->getWidth(), colourRt->getHeight() );
	context->bindWholeProgram( program );
	context->getConstantCache().updateGPU( program );
	context->getConstantCache().bind();

	if( clear ) {
		float colClr[4] = { 0.3f, 0.3f, 0.3f, 1.0f };
		glClearBufferfv( GL_COLOR, 0, colClr );
		glClearBufferfi( GL_DEPTH_STENCIL, 0, 1.0, 0 );
	}

	glEnable( GL_DEPTH_TEST );
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
}

void ForwardPipeline::unbind( Scene::RenderContext* rc ) {
	Gl::RenderContext* context = (Gl::RenderContext*) rc;

	context->popDebugMarker();
	context->getConstantCache().unbind();
}

void ForwardPipeline::conditionWob( const char* cname, Scene::WobResource* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	ForwardPipelineDataStore* pds =  CORE_NEW ForwardPipelineDataStore();
	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY ForwardPipelineDataStore::PerMaterial[pds->numMaterials] );
	Gl::WobBackEnd* backEnd = (Gl::WobBackEnd*) wob->backEnd.get();
	backEnd->pipelineDataStores[pipelineIndex] = std::unique_ptr<ForwardPipelineDataStore>(pds);

	const std::string name( cname );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		ForwardPipelineDataStore::PerMaterial* mds = &pds->materials[i];

		mds->name = name + std::string( mat->pName.p );
		const int indexSize = (mat->uiFlags & WobMaterial::WM_32BIT_INDICES) ? sizeof(uint32_t) : sizeof(uint16_t);

		// index buffer
		DataBuffer::CreationStruct indbcs = {
			DBCF_IMMUTABLE, DBT_INDEX, mat->uiNumIndices * indexSize, mat->pIndexData.p
		};

		DataBufferHandlePtr indexBuffer = DataBufferHandle::create( (mds->name + "_ib").c_str(), &indbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		mds->numIndices = mat->uiNumIndices;
		mds->indexType = (indexSize == sizeof(uint32_t)) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
		mds->vacs.elementCount = mat->numVertexElements;
		mds->vacs.indexBuffer = indexBuffer;
		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.elements[i].type = (VAO_TYPE) mat->pElements.p[i].uiType; // identical at the moment
			mds->vacs.elements[i].usage = (VAO_ELEMENT) mat->pElements.p[i].uiUsage; // identical at the moment
		}
		size_t vertexSize = Vao::getVertexSize( mds->vacs.elementCount, mds->vacs.elements );

		DataBuffer::CreationStruct vdbcs = {
			DBCF_IMMUTABLE, DBT_VERTEX, mat->uiNumVertices *  vertexSize, mat->pVertexData.p
		};
		DataBufferHandlePtr vertexBuffer = DataBufferHandle::create( (mds->name + "_vb").c_str(), &vdbcs, Core::RESOURCE_FLAGS::RMRF_NONE );

		for( int i = 0; i < mds->vacs.elementCount; ++i ) { 
			mds->vacs.data[i].buffer = vertexBuffer;
			mds->vacs.data[i].offset = Vao::AUTO_OFFSET;
			mds->vacs.data[i].stream = 0;
			mds->vacs.data[i].stride = Vao::AUTO_STRIDE;
		}
		mds->vaoHandle = 0; // defer till renderomg
	}
}
void ForwardPipeline::display( Scene::RenderContext* rc, int backWidth, int backHeight ) {
	RenderContext* context = (RenderContext*) rc;

	TexturePtr colourRt = colourRtHandle.acquire();
	context->useAsRenderTargets( 1, { &colourRt } );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
	GL_CHECK
//	glViewport( 0, 0, backWidth, backHeight );
//	GL_CHECK
	// Thikn NV's linux linear blit code is buggered, claims the linear filtering + non
	// equal rect are bad... mmmoookkkayyy!
	glBlitFramebuffer(	0, 0, colourRt->getWidth(), colourRt->getHeight(),
						0, 0, colourRt->getWidth(), colourRt->getHeight(),
						GL_COLOR_BUFFER_BIT, GL_LINEAR );
	GL_CHECK
}


void ForwardPipelineDataStore::render( Scene::RenderContext* rc ) {

	RenderContext* context = (RenderContext*) rc;
	context->getConstantCache().updateGPUObjectOnly();

	for( int i = 0;i < numMaterials; ++i ) {
		const PerMaterial* mds = &materials[i];
		if( mds->vaoHandle == 0 ) {
			PerMaterial* mdsw = const_cast<PerMaterial*>(mds);
			// defer creation, as vao have to be created sync which required acquire and could force other resource to be acquired
			mdsw->vaoHandle = VaoHandle::create( (mds->name + Vao::genEleString(mds->vacs.elementCount, mds->vacs.elements)).c_str(), &mds->vacs );
		}

		VaoPtr vao = mds->vaoHandle->tryAcquire();
		if( !vao ) {
//			LOG(INFO) << "vao not ready\n";
			return;
		}
		DataBufferPtr ib = mds->vacs.indexBuffer->tryAcquire();
		if( !ib ) {
//			LOG(INFO) << "ib not ready\n";
			return;
		}

		glBindVertexArray( vao->getName() );
		GL_CHECK
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->getName() );
		GL_CHECK
		glDrawElements( GL_TRIANGLES, mds->numIndices, mds->indexType, 0 );
		GL_CHECK
	}

}

ForwardPipelineDataStore::~ForwardPipelineDataStore() {
	for( int i = 0;i < numMaterials; ++i ) {
		ForwardPipelineDataStore::PerMaterial* mds = &materials[i];

		if( mds->vacs.indexBuffer )
			mds->vacs.indexBuffer->close();

		if( mds->vacs.data[0].buffer )
			mds->vacs.data[0].buffer->close();

		if(mds->vaoHandle)
			mds->vaoHandle->close();
	}
}

}
