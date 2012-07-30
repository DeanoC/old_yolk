#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\scenecapturepipeline.h
///
/// \brief	Experimental fragment capture renderer
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( WIERD_GL_SCENE_CAPTURE_PIPELINE_H__ )
#define WIERD_GL_SCENE_CAPTURE_PIPELINE_H__

#include "scene/pipeline.h"
#include "texture.h"
#include "vao.h"
#include "xbo.h"
#include "cl/image.h"
#include "cl/kernel.h"
#include "cl/cmdqueue.h"
#include "core/resources.h"

namespace Scene {
	class RenderContext;	
}

namespace Gl {
	namespace GPUConstants {
		struct Light;
	};
#if 0
	class SceneCapturePipeline : public Scene::Pipeline {
	public:
		friend class Gfx;

		virtual const char* getName() const { return "scene capture"; };

		virtual void bind( Scene::RenderContext* _context );
		virtual void unbind();

		virtual int getGeomPassCount() { return 1; }
		virtual void startGeomPass( int i ) {};
		virtual void endGeomPass ( int i ) {};

		virtual void display( Scene::RenderContext* context, int backWidth, int backHeight );
		virtual void merge( Scene::RenderContext* rc ){};

		virtual void conditionWob( const char* name, struct Scene::WobResource* wob );


	protected:
		SceneCapturePipeline( size_t index );
		virtual ~SceneCapturePipeline();

		Core::ScopedResourceHandle<TextureHandle>		colourRtHandle;
		Core::ScopedResourceHandle<Cl::ImageHandle>	colourClRtHandle;
		Core::ScopedResourceHandle<TextureHandle>		counterRtHandle;
		Core::ScopedResourceHandle<Cl::ImageHandle>	counterClRtHandle;
		Core::ScopedResourceHandle<TextureHandle>		minMaxRtHandle;
		Core::ScopedResourceHandle<Cl::ImageHandle>	minMaxClRtHandle;

		Core::ScopedResourceHandle<DataBufferHandle>	fragmentsBufferHandle;
		Core::ScopedResourceHandle<TextureHandle>		fragmentsTexHandle;
		Core::ScopedResourceHandle<Cl::BufferHandle>	fragmentsClBufferHandle;
		Core::ScopedResourceHandle<DataBufferHandle>	headsBufferHandle;
		Core::ScopedResourceHandle<TextureHandle>		headsTexHandle;
		Core::ScopedResourceHandle<Cl::BufferHandle>	headsClBufferHandle;

		Core::ScopedResourceHandle<DataBufferHandle>	atomicCounterHandle;
		Core::ScopedResourceHandle<DataBufferHandle>	atomicCounterClearerHandle;

		Core::ScopedResourceHandle<Cl::BufferHandle>	zOccupiedBitsHandle;
		Core::ScopedResourceHandle<Cl::BufferHandle>	lightsHandle;
		
		Core::ScopedResourceHandle<Cl::KernelHandle>	countFragmentsHandle;
		Core::ScopedResourceHandle<Cl::KernelHandle>	countFragmentChainsHandle;
		Core::ScopedResourceHandle<Cl::KernelHandle>	debugZOccupiedBitsHandle;
		Core::ScopedResourceHandle<Cl::KernelHandle>	lightTestHandle;

		Core::ScopedResourceHandle<ProgramHandle>		sceneCaptureProgramHandle;
		Core::ScopedResourceHandle<Cl::KernelHandle>	bitPackFragmentChainsHandle;
		Core::ScopedResourceHandle<Cl::KernelHandle>	headCleanerHandle;

		Core::ScopedResourceHandle<DataBufferHandle>	primitivesBufferHandle;
		Core::ScopedResourceHandle<TextureHandle>		primitivesTexHandle;
		Core::ScopedResourceHandle<Cl::BufferHandle>	primitivesClBufferHandle;

		int													numLights;
		boost::scoped_array< GPUConstants::Light >			lightsMem;
		Cl::Event											lightsUploadedEvent;

		size_t												pipelineIndex;

		Cl::Context*										contextCl;
	};

	class SceneCapturePipelineDataStore : public Scene::PipelineDataStore {
	public:
		friend class SceneCapturePipeline;
		~SceneCapturePipelineDataStore();

		virtual void render( Scene::RenderContext* context );

	protected:
		struct PNVertex {
			float pos[3];
			float norm[3];
		};

		Vao::CreationStruct			vacs; // index and vertex buffer handles are inside
		VaoHandlePtr				vaoHandle;
		size_t						numIndices;
		int							indexType;
	};
#endif
}

#endif