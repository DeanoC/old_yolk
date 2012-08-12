#pragma once
#if !defined(WIERD_GL_RENDERCONTEXT_H_)
#define WIERD_GL_RENDERCONTEXT_H_

#include "core/frustum.h"
#include "memory.h"
#include "texture.h"
#include "program.h"
#include "scene/constantcache.h"
#include "scene/rendercontext.h"

namespace Gl {

class Fbo;
class ProgramPipelineObject;

class RenderContext : public Scene::RenderContext {
public:
	friend class Gfx;

	RenderContext(void);
	~RenderContext(void);

	//---------------------------------------------------------------

	virtual void pushDebugMarker( const char* text ) const override;
	virtual void popDebugMarker() const override;
	virtual void setCamera( const Scene::CameraPtr& _cam ) override;
	virtual void threadActivate() override; 	// must be called once before this context on a particular thread
	virtual void prepToRender() override; // used to indicate this render context is used for rendering versus loading/non visual render stuff
	virtual void bindConstants() override;
	virtual void swapBuffers() override;
	virtual void reset() override;
	virtual void bindProgram( const Scene::ProgramPtr& prg ) override;
	virtual void unbindProgram() override;

	virtual void useAsRenderTargets( const Scene::TexturePtr& pTarget, const Scene::TexturePtr& pDepthTarget ) override;
	virtual void useAsRenderTarget( Scene::TexturePtr pTarget ) override;
	virtual void useAsDepthOnlyRenderTargets( Scene::TexturePtr pDepthTarget ) override;
	virtual void useAsRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets, const Scene::TexturePtr& pDepthTarget ) override;
	virtual void useAsRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const pTargets ) override;
	virtual void useNoRenderTargets() override;

	//---------------------------------------------------------------



	void setShader( const Scene::ProgramPtr& prg );
	void resetShader( PROGRAM_TYPE type );

	// use this pipeline instead of the context pipeline
	void bindPipeline( ProgramPipelineObject* pPPO );
	// return the pipeline to the render context own one
	void unbindPipeline();
	ProgramPipelineObject* getCurrentPpo() const { return curPpo; }

	Fbo* getFbo() const { return fbo.get(); };

#if PLATFORM == WINDOWS
	// if any windows call need GLRC or DC grab from here (not cross platform obviously...)
	HDC											hDC;
	HGLRC										hRC;
#else
	void*										x11Display;
	unsigned long								x11Window;
	void*										glxContext;
#endif
private:
#if PLATFORM == WINDOWS
	// context must be created on main thread
	void setGlContext( HDC _hDC, HGLRC _hRC );
#else
	// context must be created on main thread
	void setGlContext( void* dis, unsigned long win, void* ctx );
#endif

	ProgramPipelineObject*						curPpo;
	boost::scoped_ptr<ProgramPipelineObject>	defaultPpo;
	boost::scoped_ptr<Fbo>						fbo;
	boost::scoped_array<Scene::TexturePtr>		boundRenderTargets;
	bool										debugOutputInstalled;


};

}

#endif