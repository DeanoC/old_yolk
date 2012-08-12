#pragma once
//!-----------------------------------------------------
//!
//! \file rendercontext.h
//! simple redirects to the backend version
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_RENDERCONTEXT_H_)
#define YOLK_SCENE_RENDERCONTEXT_H_

#include "scene/camera.h"
#include "scene/program.h"
#include "scene/constantcache.h"
#include "scene/texture.h"

namespace Scene {

	class RenderContext {
	public:
		virtual void pushDebugMarker( const char* ) const = 0;
		virtual void popDebugMarker() const = 0;		

		virtual void setCamera( const CameraPtr& _cam ) = 0;
		virtual void threadActivate() = 0; 	// must be called once, before this context is used on a particular thread
		virtual void prepToRender() = 0; // used to indicate this render context is used for rendering versus loading/non visual render stuff

		virtual void bindConstants() = 0;

		virtual void swapBuffers() = 0;
		virtual void reset() = 0;
		virtual void bindProgram( const Scene::ProgramPtr& prg ) = 0;
		virtual void unbindProgram() = 0;

		virtual void useAsRenderTargets( const TexturePtr& pTarget, const TexturePtr& pDepthTarget ) = 0;
		virtual void useAsRenderTarget( TexturePtr pTarget ) = 0;
		virtual void useAsDepthOnlyRenderTargets( TexturePtr pDepthTarget ) = 0;
		virtual void useAsRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets, const TexturePtr& pDepthTarget ) = 0;
		virtual void useAsRenderTargets( unsigned int numTargets, const TexturePtr* const pTargets ) = 0;
		virtual void useNoRenderTargets() = 0;


		Scene::ConstantCache& getConstantCache() { return *constantCache; }
		const Scene::ConstantCache& getConstantCache() const { return *constantCache; }
	
		const Scene::CameraPtr getCamera() { return constantCache->getCamera(); }
		const Core::Frustum* getFrustum() { return viewFrustum; }

	protected:
		boost::scoped_ptr<Scene::ConstantCache>		constantCache;
		// note in most cases frustum and camera match, but in some debug cases they
		// might not (to allow you see how frustum culling is working or not)
		const Core::Frustum*						viewFrustum;
		boost::scoped_array<Scene::ProgramPtr>		boundPrograms;

	};
}

#endif // end YOLK_SCENE_RENDERCONTEXT_H_