#include "ogl.h"
#include "core/resources.h"
#include "scene/wobfile.h"
#include "scene/imagecomposer.h"
#include "gfx.h"
#include "shaderman.h"
#include "texture.h"
#include "databuffer.h"
#include "vao.h"
#include "fbo.h"
#include "rendercontext.h"
#include "wobbackend.h"
#include "vtpipeline.h"

// compute renderer needs CL
//#include "cl/ocl.h"
//#include "cl/platform.h"
//#include "cl/kernel.h"
//#include "cl/image.h"
//#include "cl/programman.h"
//#include "cl/buffer.h"


namespace Gl {

static const int binWidth = 512;
static const int binHeight = 256;

enum ScratchMemLocations {
	SML_FRAG_ALLOCATOR = 0,
	SML_FACE_ALLOCATOR,
	SML_PRIMITIVE_ID_COUNTER,

	SML_MAX_LOCATIONS
};

static const uint32_t scratchInit[SML_MAX_LOCATIONS] = {
	0, // SML_FRAG_ALLOCATOR
	0, // SML_FACE_ALLOCATOR
	0, // SML_PRIMITIVE_ID_COUNTER
};

void VtPipeline::createCaptureBuffers() {
	using namespace Scene;
	//---------------------------------------
	// frag count
	Texture::CreationStruct bcrt = {
		TCF_2D | TCF_RENDER_TARGET, GTF_R8, 
		1, targetWidth, targetHeight, 0, 0
	};
	fragCountRtHandle.reset( TextureHandle::create( "_vtpipe_fragcount_rt", &bcrt ) );

	//---------------------------------------
	// frag header
	Texture::CreationStruct bhrt = {
		TCF_2D | TCF_RENDER_TARGET, GTF_R32F,
		1, targetWidth, targetHeight, 0, 0
	};
	fragHeaderRtHandle.reset( TextureHandle::create( "_vtpipe_fragheader_rt", &bhrt ) );

	//---------------------------------------
	// pixel temp 
	Texture::CreationStruct pptrt = {
		TCF_2D | TCF_RENDER_TARGET, GTF_R32F,
		1, targetWidth, targetHeight, 0, 0
	};
	pixelTempRtHandle.reset( TextureHandle::create( "_vtpipe_pixeltemp_rt", &pptrt ) );

	//---------------------------------------
	// scratch texture
	// to be used by atomic read/write as a form of scratch memory
	// each frame is reset (via a GPU copy) to its initial settings
	DataBuffer::CreationStruct scratchics = {
		DBCF_IMMUTABLE, DBT_GENERAL, 
		sizeof( uint32_t ) * SML_MAX_LOCATIONS, 
		scratchInit
	};
	scratchInitHandle.reset( DataBufferHandle::create( "_vtpipe_scratchinit_db", &scratchics ) );
	DataBuffer::CreationStruct scratchcs = {
		DBCF_GPU_COPY, DBT_TEXTURE, scratchics.size,
	};
	scratchBufHandle.reset( DataBufferHandle::create( "_vtpipe_scratch_db", &scratchcs ) );
	auto scratchBuf = std::static_pointer_cast<Gl::DataBuffer>( scratchBufHandle.acquire() );
	Texture::CreationStruct scratchtcs = {
		TCF_BUFFER, GTF_R32UI,
		1, 1, 1, 1, 1, scratchBuf->getName()
	};
	scratchTexHandle.reset( TextureHandle::create( "_vtpipe_scratch_tex", &scratchtcs ) );

	//---------------------------------------
	// fragment buffer and texture and CL buffer (all same memory)
	static const int MAX_FRAGMENTS = 10 * 1024 * 1024 * (128/8); // each fragment is 128 bits (4xuint32_t)
	DataBuffer::CreationStruct fbcs = {
		DBCF_NONE, DBT_TEXTURE, MAX_FRAGMENTS, nullptr
	};
	fragmentsBufferHandle.reset( DataBufferHandle::create( "_vtpipe_binfrags_db", &fbcs ) );

	auto fragmentsBuffer = std::static_pointer_cast<Gl::DataBuffer>( fragmentsBufferHandle.acquire() );
	Texture::CreationStruct fragcs = {
		TCF_BUFFER, GTF_RGBA32UI, 1, 1, 1, 1, 1, fragmentsBuffer->getName()
	};
	fragmentsTexHandle.reset( TextureHandle::create( "_vtpipe_binfrags_tex", &fragcs ) );

/*	Cl::Buffer::CreationStruct clfbcs = {
		contextCl,
		Cl::BCF_FROM_GL | Cl::BCF_KERNEL_READ,
		0, nullptr, fragmentsBufferHandle.acquire()
	};
	fragmentsClBufferHandle.reset( Cl::BufferHandle::create( "_vtpipe_binfrags_cl_buf", &clfbcs ) );*/

	//---------------------------------------
	// geometry capture buffer, texture and CL buffer 
	// (all same memory) if too much memory could move to indexed
	// vertices's and/or take advantage of quads...

	// each face is 384 bits (16 x float), 4 are unused currently	
	static const int MAX_FACES = 2 * 1024 * 1024 * (384/8);
	DataBuffer::CreationStruct facescs = {
		DBCF_NONE, DBT_TEXTURE, MAX_FACES, nullptr
	};
	facesBufferHandle.reset( DataBufferHandle::create( "_vtpipe_faces_db", &facescs ) );

	auto facesBuffer = std::static_pointer_cast<Gl::DataBuffer>( facesBufferHandle.acquire() );
	Texture::CreationStruct facestcs = {
		TCF_BUFFER, GTF_RGBA32F, 1, 1, 1, 1, 1, facesBuffer->getName()
	};
	facesTexHandle.reset( TextureHandle::create( "_vtpipe_faces_tex", &facestcs ) );

/*	Cl::Buffer::CreationStruct clfabcs = {
		contextCl,
		Cl::BCF_FROM_GL | Cl::BCF_KERNEL_READ,
		0, nullptr, facesBufferHandle.acquire()
	};
	facesClBufferHandle.reset( Cl::BufferHandle::create( "_vtpipe_faces_cl_buf", &clfabcs ) );
*/
	// programs
	fragCountProgramHandle.reset( ProgramHandle::create( "adder" ) );
	fragHeaderProgramHandle.reset( ProgramHandle::create( "alloc32" ) );
	captureFragmentsProgramHandle.reset( ProgramHandle::create( "capture_fragments" ) );

}
int VtPipeline::getGeomPassCount() { 
	return 1;//NUM_GEOM_PASSES; 
}

void VtPipeline::startFragCountGeomPass() {
	using namespace Scene;

	// we hijack this pass for the actual geometry
	// capture, for now I use imageStore in the geometry
	// shader as this is portable across all GL4.2 cards
	// but using AMD_transform_feedback3_lines_triangles
	// extension would allow us to use transform feedback
	// which might be faster? TODO add other path and 
	// profile (runtime?) to use the best

	//----------------------------
	// reset scratch frame statics via gpu -> gpu copy
	auto initer = std::static_pointer_cast<Gl::DataBuffer>( scratchInitHandle.acquire() );
	auto scratch = std::static_pointer_cast<Gl::DataBuffer>( scratchBufHandle.acquire() );
	glBindBuffer( GL_COPY_READ_BUFFER, initer->getName() );
	glBindBuffer( GL_COPY_WRITE_BUFFER, scratch->getName() );
	glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, initer->getSize() );
	GL_CHECK
	//----------------------------

	//----------------------------
	// a full geometry pass captures fragment counts of all geometry
	TexturePtr fcRt = fragCountRtHandle.acquire();
	ProgramPtr program = fragCountProgramHandle.acquire();

	const uint32_t targetDims[4] = { targetWidth, targetHeight, 0, 0 };
	context->getConstantCache().setUIVector( Scene::CVN_TARGET_DIMS, targetDims );
/*
	auto scratchTex = scratchTexHandle.acquire();
	glBindImageTexture( 0, scratchTex->getName(), 0, 
					GL_FALSE, 0, GL_READ_WRITE, GL_R32UI );
	GL_CHECK

	auto facesTex = facesTexHandle.acquire();
	glBindImageTexture( 1, facesTex->getName(), 0, 
					GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );
	GL_CHECK
*/
	context->useAsRenderTarget( fcRt );
	context->bindProgram( program );
	context->getConstantCache().updateGPU( ); //program );
	context->bindConstants();

	float colClr[4] = { 0.1, 0, 0, 0 };
	glClearBufferfv( GL_COLOR, 0, colClr );

	glDisable( GL_DEPTH_TEST );
	glEnable( GL_BLEND );
	glBlendEquationSeparate( GL_FUNC_ADD, GL_FUNC_ADD );
	glBlendFuncSeparate( GL_ONE, GL_ONE, GL_ONE, GL_ONE );

}

void VtPipeline::endFragCountGeomPass() {
/*	DataBufferPtr dummyVBO = dummyVBOHandle.acquire();
	VaoPtr dummyVao = dummyVaoHandle.acquire();

	const uint32_t targetDims[4] = { binWidth, binHeight, 0, 0 };
	context->getConstantCache().setUIVector( CVN_TARGET_DIMS, targetDims );

	auto scratchTex = scratchTexHandle.acquire();
	glBindImageTexture( 0, scratchTex->getName(), 0, 
					GL_FALSE, 0, GL_READ_WRITE, GL_R32UI );
	GL_CHECK
	auto fragCountTex = fragCountRtHandle.acquire();
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, fragCountTex->getName() );
	GL_CHECK

	auto fragHeaderRt = fragHeaderRtHandle.acquire();
	context->useAsRenderTarget( fragHeaderRt );

	ProgramPtr program = fragHeaderProgramHandle.acquire();
	context->bindWholeProgram( program );

	context->getConstantCache().updateGPU( program );
	context->bindConstants();

	// draw quad write only
	glDisable( GL_BLEND );
	glBindBuffer( GL_ARRAY_BUFFER, dummyVBO->getName() );
	glBindVertexArray( dummyVao->getName() );
 	glDrawArrays( GL_POINTS, 0, 1 );
	GL_CHECK
*/
}

void VtPipeline::startCaptureFragmentsGeomPass() {
	auto dummyVBO = std::static_pointer_cast<Gl::DataBuffer>( dummyVBOHandle.acquire() );
	auto dummyVao = std::static_pointer_cast<Gl::Vao>( dummyVaoHandle.acquire() );

	// clear per pixel temp to keep count of current fragment count
	auto pixelTempRt = std::static_pointer_cast<Gl::Texture>( pixelTempRtHandle.acquire() );
	context->useAsRenderTarget( pixelTempRt );
	float colClr[4] = { 0, 0, 0, 0 };
	glClearBufferfv( GL_COLOR, 0, colClr );

	auto fragHeaderTex = std::static_pointer_cast<Gl::Texture>( fragHeaderRtHandle.acquire() );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, fragHeaderTex->getName() );
	GL_CHECK

	glBindImageTexture( 0, pixelTempRt->getName(), 0, 
					GL_FALSE, 0, GL_READ_WRITE, GL_R32UI );
	GL_CHECK
	auto fragmentsTex = std::static_pointer_cast<Gl::Texture>( fragmentsTexHandle.acquire() );
	glBindImageTexture( 1, fragmentsTex->getName(), 0, 
					GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32UI );
	GL_CHECK
	auto scratchTex = std::static_pointer_cast<Gl::Texture>( scratchTexHandle.acquire() );
	glBindImageTexture( 2, scratchTex->getName(), 0, 
					GL_FALSE, 0, GL_READ_WRITE, GL_R32UI );
	GL_CHECK

	Scene::ProgramPtr program = captureFragmentsProgramHandle.acquire();
	context->bindProgram( program );
	context->getConstantCache().updateGPU( program );
	context->bindConstants();

	// bind a fake render target we never actually read or write it
	// as we use image load / stores
	auto depthRt = std::static_pointer_cast<Gl::Texture>( depthRtHandle.acquire() );
	context->useAsDepthOnlyRenderTargets( depthRt );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	GL_CHECK

}

}