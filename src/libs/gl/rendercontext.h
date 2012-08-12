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

	//---------------------------------------------------------------
	void swapBuffers();

	// reset to default
	void reset();

	void useAsRenderTargets( const TexturePtr& pTarget, const TexturePtr& pDepthTarget );
	void useAsRenderTarget( TexturePtr pTarget );
	void useAsDepthOnlyRenderTargets( TexturePtr pDepthTarget );
	void useAsRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets, const TexturePtr& pDepthTarget );
	void useAsRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets );
	void useNoRenderTargets();

	// these set programs into the current pipeline
	void setProgram( const Scene::ProgramPtr& prg );
	void resetProgram( PROGRAM_TYPE type );

	// use this pipeline instead of the context pipeline
	void bindPipeline( ProgramPipelineObject* pPPO );
	// return the pipeline to the render context own one
	void unbindPipeline();

	// replace the pipeline with a whole program (all shaders linked into one)
	void bindWholeProgram( const Scene::ProgramPtr& prg );
	// return to render context pipeline 
	void unbindWholeProgam();


	Scene::ConstantCache& getConstantCache() { return *constantCache; }
	const Scene::ConstantCache& getConstantCache() const { return *constantCache; }

	ProgramPipelineObject* getCurrentPpo() const { return curPpo; }

	Fbo* getFbo() const { return fbo.get(); };

	const Scene::CameraPtr getCamera() { return constantCache->getCamera(); }

	const Core::Frustum* getFrustum() { return viewFrustum; }

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
	boost::scoped_ptr<Scene::ConstantCache>		constantCache;
	// note in most cases frustum and camera match, but in some debug cases they
	// might not (to allow you see how frustum culling is working or not)
	const Core::Frustum*						viewFrustum;
	boost::scoped_ptr<Fbo>						fbo;
	boost::scoped_array<TexturePtr>				boundRenderTargets;
	boost::scoped_ptr<ProgramPipelineObject>	ppo;
	boost::scoped_array<Scene::ProgramPtr>		boundPrograms;
	ProgramPipelineObject*						curPpo;
	bool										debugOutputInstalled;

};

}

#endif