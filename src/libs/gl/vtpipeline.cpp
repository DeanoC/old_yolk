//!-----------------------------------------------------
//!
//! @file vtpipeline.cpp
//! @brief A pipeline that represent the games renderer
//!
//!-----------------------------------------------------

#include "gl.h"
#include "core/resourceman.h"
#include "scene/wobfile.h"
#include "gfx.h"
#include "texture.h"
#include "databuffer.h"
#include "vao.h"
#include "fbo.h"
#include "rendercontext.h"
#include "wobbackend.h"
#include "imagecomposer.h"
#include "cl/platform.h"

#include "vtpipeline.h"

namespace Gl {

VtPipeline::VtPipeline( size_t index ) :
	pipelineIndex( index )
{
	contextCl = Cl::Platform::get()->getPrimaryContext().get();

	targetWidth = Gfx::get()->getScreenWidth();
	targetHeight = Gfx::get()->getScreenHeight();
	targetSamples = 8;

	Texture::CreationStruct fpcrt = {
		TCF_2D | TCF_RENDER_TARGET | TCF_MULTISAMPLE,
		TF_RGBA8888, 
		1, targetWidth, targetHeight, 0, 0, targetSamples
	};
	colourRtHandle.reset( TextureHandle::create( "_vtpipe_colrt", &fpcrt ) );

	Texture::CreationStruct fpdrt = {
		TCF_2D | TCF_RENDER_TARGET | TCF_MULTISAMPLE,
		GL_DEPTH24_STENCIL8, 
		1, targetWidth, targetHeight, 0, 0, targetSamples
	};
	depthRtHandle.reset( TextureHandle::create( "_vtpipe_depthrt", &fpdrt ) );

	DataBuffer::CreationStruct fpdvb = {
		DBCF_NONE, DBT_VERTEX, 0, nullptr
	};
	dummyVBOHandle.reset( DataBufferHandle::create( "_vtpipe_dummy_vb", &fpdvb ) );
	Vao::CreationStruct fpdvoa = { 0 };
	dummyVaoHandle.reset( VaoHandle::create( "_vtpipe_dummy_vao", &fpdvoa ) );

	mainProgramHandle.reset( ProgramHandle::create( "flat_basic" ) );
	resolve8msaaProgramHandle.reset( ProgramHandle::create( "resolve8msaa" ) );
	debugCaptureFragmentsProgramHandle.reset( ProgramHandle::create( "debug_captured_fragments" ) );

	createCaptureBuffers();
}

VtPipeline::~VtPipeline() {
}

void VtPipeline::bind( Scene::RenderContext* rc ) {
	context = (Gl::RenderContext*) rc;
	context->pushDebugMarker( "VtPipeline" );

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
}

void VtPipeline::startMainGeomPass() {
	TexturePtr colourRt = colourRtHandle.acquire();
	TexturePtr depthRt = depthRtHandle.acquire();
	ProgramPtr program = mainProgramHandle.acquire();

	context->useAsRenderTargets( colourRt, depthRt );
	context->bindWholeProgram( program );
	context->getConstantCache().updateGPU( program );
	context->getConstantCache().bind();

	float colClr[4] = { 0, 0, 0, 1.0f };
	glClearBufferfv( GL_COLOR, 0, colClr );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glDisable( GL_BLEND );
}

void VtPipeline::unbind() {
	context->popDebugMarker();
	context->getConstantCache().unbind();

	context = nullptr;

}

void VtPipeline::conditionWob( const char* cname, Scene::WobResource* wob ) {
	using namespace Scene;
	WobFileHeader* header = wob->header.get();

	VtPipelineDataStore* pds =  CORE_NEW VtPipelineDataStore();
	pds->numMaterials = header->uiNumMaterials;
	pds->materials.reset( CORE_NEW_ARRAY VtPipelineDataStore::PerMaterial[pds->numMaterials] );
	Gl::WobBackEnd* backEnd = (Gl::WobBackEnd*) wob->backEnd.get();
	backEnd->pipelineDataStores[pipelineIndex] = std::unique_ptr<VtPipelineDataStore>(pds);

	const std::string name( cname );

	for( uint16_t i = 0; i < header->uiNumMaterials; ++i ) {
		WobMaterial* mat = &header->pMaterials.p[i];

		VtPipelineDataStore::PerMaterial* mds = &pds->materials[i];

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

void VtPipeline::display( Scene::RenderContext* rc, int backWidth, int backHeight ) {
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

void VtPipeline::merge( Scene::RenderContext* rc ) {
//	composer->putTexture( colourRtHandle.get(), ImageComposer::ALPHA_BLEND, 
//		Math::Vector2(0,0), Math::Vector2(1,1), 
//		Core::RGBAColour::unpackARGB(0xFFFFFFFF), 3 );
	RenderContext* context = (RenderContext*) rc;
	DataBufferPtr dummyVBO = dummyVBOHandle.acquire();
	VaoPtr dummyVao = dummyVaoHandle.acquire();

	if( 0 ) {
		TexturePtr colourRt = colourRtHandle.acquire();
		ProgramPtr program = resolve8msaaProgramHandle.acquire();

		// setup resolve program, textures and constants
		context->bindWholeProgram( program );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, colourRt->getName() );
	} else {
//		TexturePtr fragHeaderTex = fragHeaderRtHandle.acquire();
//		TexturePtr fragmentsTex = fragmentsTexHandle.acquire();
//		glActiveTexture( GL_TEXTURE1 );
//		glBindTexture( GL_TEXTURE_2D, fragHeaderTex->getName() );
//		glActiveTexture( GL_TEXTURE2 );
//		glBindTexture( GL_TEXTURE_BUFFER, fragmentsTex->getName() );

	}

	ProgramPtr program = debugCaptureFragmentsProgramHandle.acquire();
	context->bindWholeProgram( program );
struct 	context->getConstantCache().bind();

	glBindBuffer( GL_ARRAY_BUFFER, dummyVBO->getName() );
	glBindVertexArray( dummyVao->getName() );

	TexturePtr fragCountTex = fragCountRtHandle.acquire();
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, fragCountTex->getName() );

	// draw quad
 	glDrawArrays( GL_POINTS, 0, 1 );

	TexturePtr colourRt = colourRtHandle.acquire();
	context->useAsRenderTargets( 1, { &colourRt } );

}

void VtPipelineDataStore::render( Scene::RenderContext* rc ) {

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
		if( !vao ) {/*LOG(INFO) << "vao not ready\n";*/	return; }
		DataBufferPtr ib = mds->vacs.indexBuffer->tryAcquire();
		if( !ib ) { /*LOG(INFO) << "ib not ready\n";*/return; }

		glBindVertexArray( vao->getName() );
		GL_CHECK
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ib->getName() );
		GL_CHECK
		glDrawElements( GL_TRIANGLES, mds->numIndices, mds->indexType, 0 );
		GL_CHECK
	}

}

VtPipelineDataStore::~VtPipelineDataStore() {
	for( int i = 0;i < numMaterials; ++i ) {
		VtPipelineDataStore::PerMaterial* mds = &materials[i];

		if( mds->vacs.indexBuffer )
			mds->vacs.indexBuffer->close();

		if( mds->vacs.data[0].buffer )
			mds->vacs.data[0].buffer->close();

		if(mds->vaoHandle)
			mds->vaoHandle->close();
	}
}

void VtPipeline::startGeomPass( int i ) {
	switch( i ) {
		case FRAG_COUNTER:
		context->pushDebugMarker( "VtPipeline::BinCounterPass" );
		startFragCountGeomPass();
		break;

		case CAPTURE_FRAGMENTS:
		context->pushDebugMarker( "VtPipeline::CaptureFragmentsPass" );
		startCaptureFragmentsGeomPass();
		break;

		case MAIN_PASS:
		context->pushDebugMarker( "VtPipeline::MainPass" );
		startMainGeomPass();
		break;

	}
}

void VtPipeline::endGeomPass ( int i ) {
	switch( i ) {
		case FRAG_COUNTER: endFragCountGeomPass(); break;		
		default: break;
	}
	context->popDebugMarker();
}


}