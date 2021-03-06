#include "ogl.h"
#if PLATFORM == WINDOWS
#else
#include "glxew.h"
#endif

#include "gl/texture.h"
#include "gl/fbo.h"
#include "gl/programpipelineobject.h"
#include "gl/databuffer.h"
#include "scene/constantcache.h"
#include "rendercontext.h"

namespace Gl {

void debugOutput (
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    GLvoid* userParam ) {

	char debSource[32], debType[32];

    if(source == GL_DEBUG_SOURCE_API_ARB)
            strcpy(debSource, "OpenGL");
    else if(source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
            strcpy(debSource, "Windows");
    else if(source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
            strcpy(debSource, "Shader Compiler");
    else if(source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
            strcpy(debSource, "Third Party");
    else if(source == GL_DEBUG_SOURCE_APPLICATION_ARB)
            strcpy(debSource, "Application");
    else if(source == GL_DEBUG_SOURCE_OTHER_ARB)
            strcpy(debSource, "Other");

    if(type == GL_DEBUG_TYPE_ERROR_ARB)
            strcpy(debType, "error");
    else if(type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
            strcpy(debType, "deprecated behavior");
    else if(type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
            strcpy(debType, "undefined behavior");
    else if(type == GL_DEBUG_TYPE_PORTABILITY_ARB)
            strcpy(debType, "portability");
    else if(type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
            strcpy(debType, "performance");
    else if(type == GL_DEBUG_TYPE_OTHER_ARB)
            strcpy(debType, "message");

    if(severity == GL_DEBUG_SEVERITY_HIGH_ARB) {
		LOG(ERROR) << debSource << " : " << debType << " : " << id << " : " << message;

#if PLATFORM == WINDOWS 
		DebugBreak();
#else
		raise(SIGINT);
#endif
	} else if(severity == GL_DEBUG_SEVERITY_MEDIUM_ARB) {
		LOG(WARNING) << debSource << " : " << debType << " : " << id << " : " << message;
	} else {
		//if(severity == GL_DEBUG_SEVERITY_LOW_ARB)
		LOG(INFO) << debSource << " : " << debType << " : " << id << " : " << message;
	}
}
void debugOutputAMD(GLuint id, GLenum category, GLenum severity, GLsizei, const GLchar* message, GLvoid*) {
	char debType[32];

    if(category == GL_DEBUG_CATEGORY_API_ERROR_AMD )
            strcpy(debType, "error");
    else if(category == GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD)
            strcpy(debType, "Windows");
    else if(category == GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD)
            strcpy(debType, "Shader Compiler");
    else if(category == GL_DEBUG_CATEGORY_APPLICATION_AMD )
            strcpy(debType, "Application");
    else if(category == GL_DEBUG_CATEGORY_DEPRECATION_AMD)
            strcpy(debType, "deprecated behavior");
    else if(category == GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD)
            strcpy(debType, "undefined behavior");
    else if(category == GL_DEBUG_CATEGORY_PERFORMANCE_AMD)
            strcpy(debType, "performance");
    else if(category == GL_DEBUG_CATEGORY_OTHER_AMD)
            strcpy(debType, "message");

    if(severity == GL_DEBUG_SEVERITY_HIGH_AMD) {
			LOG(FATAL) << debType << " : " << id << " : " << message;
//			DebugBreak();
	} else if(severity == GL_DEBUG_SEVERITY_MEDIUM_AMD) {
			LOG(WARNING) << debType << " : " << id << " : " << message;
	} else if(severity == GL_DEBUG_SEVERITY_LOW_AMD) {
			LOG(INFO) << debType << " : " << id << " : " << message;
	}

//	LOG(INFO) << debType << " : " << id << " : " << message;
}

RenderContext::RenderContext() {
	viewFrustum = nullptr;
	debugOutputInstalled = false;

}

RenderContext::~RenderContext() {
	fbo.reset(0);
	boundPrograms.reset( 0 );
	boundRenderTargets.reset( 0 );
}

void RenderContext::threadActivate() {
#if PLATFORM == WINDOWS 
	wglMakeCurrent(hDC, hRC);
#elif PLATFORM == POSIX
	glXMakeCurrent( (Display*) x11Display, x11Window, (GLXContext) glxContext );
#endif
	GL_CHECK
}

void RenderContext::reset() {
	bindPipeline( defaultPpo.get() );
	useNoRenderTargets();
	resetShader( PT_VERTEX );
	resetShader( PT_TESS_CONTROL );
	resetShader( PT_TESS_EVAL );
	resetShader( PT_GEOMETRY );
	resetShader( PT_FRAGMENT );
}

#if PLATFORM == WINDOWS 
void RenderContext::setGlContext( HDC _hDC, HGLRC _hRC ) {
	hDC = _hDC;
	hRC = _hRC;
}
#else
void RenderContext::setGlContext( void* dis, unsigned long win, void* ctx ) {
	x11Display = dis;
	x11Window = win;
	glxContext = ctx;
}

#endif

void RenderContext::useAsRenderTargets( const Scene::TexturePtr& pTarget, const Scene::TexturePtr& pDepthTarget ) {
	useNoRenderTargets();
	fbo->attach( FAP_COLOUR0, pTarget );
	fbo->attach( FAP_DEPTH, pDepthTarget );
	boundRenderTargets[0] = pTarget;
	boundRenderTargets[FAP_MAX_ATTACHMENT_POINTS-1] = pDepthTarget;
	fbo->bind();
}
void RenderContext::useAsRenderTarget( Scene::TexturePtr pTarget ) {
	useNoRenderTargets();
	fbo->attach( FAP_COLOUR0, pTarget );
	fbo->detach( FAP_DEPTH );
	boundRenderTargets[0] = pTarget;
	fbo->bind();
}
void RenderContext::useAsDepthOnlyRenderTargets( Scene::TexturePtr pDepthTarget ) {
	useNoRenderTargets();
	fbo->attach( FAP_DEPTH, pDepthTarget );
	boundRenderTargets[FAP_MAX_ATTACHMENT_POINTS-1] = pDepthTarget;
	fbo->bind();
}

void RenderContext::useAsRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets, const Scene::TexturePtr& pDepthTarget ) {
	useNoRenderTargets();
	fbo->attach( FAP_DEPTH, pDepthTarget );
	boundRenderTargets[FAP_MAX_ATTACHMENT_POINTS-1] = pDepthTarget;
	for( unsigned int i = 0; i < numTargets; ++i ) {
		fbo->attach( (FBO_ATTACHMENT_POINT) (FAP_COLOUR0 + i), pTargets[i] );
		boundRenderTargets[i] = pTargets[i];
	}
	fbo->bind();
}

void RenderContext::useAsRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets ) {
	useNoRenderTargets();
	for( unsigned int i = 0; i < numTargets; ++i ) {
		fbo->attach( (FBO_ATTACHMENT_POINT) (FAP_COLOUR0 + i), pTargets[i] );
		boundRenderTargets[i] = pTargets[i];
	}
	fbo->bind();
}

void RenderContext::useNoRenderTargets() {
	fbo->unbind();
	for( int i = 0; i < FAP_MAX_ATTACHMENT_POINTS-1; ++i ) {
		if( boundRenderTargets[i] != NULL ) {
			fbo->detach( (FBO_ATTACHMENT_POINT)(FAP_COLOUR0 + i));
			boundRenderTargets[i] = NULL;
		}
	}
	if( boundRenderTargets[FAP_MAX_ATTACHMENT_POINTS-1] != NULL ) {
		fbo->detach( FAP_DEPTH );
		boundRenderTargets[FAP_MAX_ATTACHMENT_POINTS-1] = NULL;
	}
}

void RenderContext::setShader( const Scene::ProgramPtr& sprg ) {
	auto prg = std::static_pointer_cast<Program>( sprg );

	CORE_ASSERT( prg != nullptr );

	PPO_STAGE stage;
	switch( prg->getType() ) {
	case MNT_VERTEX_SHADER_OBJECT: stage = PPO_VERTEX_STAGE; break;
	case MNT_FRAGMENT_SHADER_OBJECT: stage = PPO_FRAGMENT_STAGE; break;
	case MNT_GEOMETRY_SHADER_OBJECT: stage = PPO_GEOMETRY_STAGE; break;
	case MNT_TESS_CONTROL_OBJECT: stage = PPO_TESS_CONTROL_STAGE; break;
	case MNT_TESS_EVAL_SHADER_OBJECT: stage = PPO_TESS_EVAL_STAGE; break;
	}
	curPpo->attach( stage, prg );
}

void RenderContext::resetShader( PROGRAM_TYPE type ) {
	PPO_STAGE stage;
	switch( type ) {
	case MNT_VERTEX_SHADER_OBJECT: stage = PPO_VERTEX_STAGE; break;
	case MNT_FRAGMENT_SHADER_OBJECT: stage = PPO_FRAGMENT_STAGE; break;
	case MNT_GEOMETRY_SHADER_OBJECT: stage = PPO_GEOMETRY_STAGE; break;
	case MNT_TESS_CONTROL_OBJECT: stage = PPO_TESS_CONTROL_STAGE; break;
	case MNT_TESS_EVAL_SHADER_OBJECT: stage = PPO_TESS_EVAL_STAGE; break;
	}
	curPpo->detach( stage );
}
void RenderContext::bindPipeline( ProgramPipelineObject* pPPO ) {
	curPpo = pPPO;
	curPpo->bind();
}

void RenderContext::unbindPipeline() {
	if( curPpo != defaultPpo.get() ) {
		curPpo->unbind();
	}
	curPpo = defaultPpo.get();
	curPpo->bind();	
}

void RenderContext::bindProgram( const Scene::ProgramPtr& sprg ) {
	auto prg = std::static_pointer_cast<Program>( sprg );

	curPpo = nullptr;
	glUseProgram( prg->getName() );
}

void RenderContext::unbindProgram() {
	glUseProgram( 0 );
	unbindPipeline();
}

void RenderContext::pushDebugMarker( const char* text ) const {
	if( glStringMarkerGREMEDY ) {
		glStringMarkerGREMEDY( 0, text );
	}
};

void RenderContext::popDebugMarker() const {
};

void RenderContext::setCamera( const Scene::CameraPtr& _cam ) {
	constantCache->setCamera( _cam );
	viewFrustum = &_cam->getFrustum();
}

void RenderContext::prepToRender() {
	using namespace Scene;
	// fbo NULL used to mark first activation
	if( fbo == NULL ) {
		GL_CHECK
		constantCache.reset( CORE_NEW Scene::ConstantCache() );
		fbo.reset( CORE_NEW Fbo() );
		defaultPpo.reset( CORE_NEW ProgramPipelineObject() );
	
		boundRenderTargets.reset( CORE_NEW_ARRAY TexturePtr[ FAP_MAX_ATTACHMENT_POINTS ] );
		boundPrograms.reset( CORE_NEW_ARRAY ProgramPtr[ PPO_MAX_STAGES ] );
	}
	GL_CHECK
	bindPipeline( defaultPpo.get() );
	GL_CHECK
}

void RenderContext::swapBuffers() {
#if PLATFORM == WINDOWS
	SwapBuffers( hDC );
#elif PLATFORM == POSIX
	glXSwapBuffers ( (Display*) x11Display, x11Window );
#endif
}

void RenderContext::bindConstants() {
	using namespace Scene;
	for( int i = 0; i < Scene::CF_USER_BLOCKS; ++i ) {
		const auto db = std::static_pointer_cast<Gl::DataBuffer>( constantCache->getBlock( (Scene::CONSTANT_FREQ_BLOCKS)i)->acquire() );
		glBindBufferBase( GL_UNIFORM_BUFFER, i, db->getName() );
		GL_CHECK
	}
}

}