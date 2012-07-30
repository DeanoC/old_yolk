#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\vtpipeline.h
///
/// \brief	Declares the controlling class for vt custom pipeline
///
/// \remark	Copyright (c) 2012 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( WIERD_GL_VT_PIPELINE_H__ )
#define WIERD_GL_VT_PIPELINE_H__

#include "core/resources.h"
#include "scene/pipeline.h"
#include "vao.h"

namespace Scene {
	class RenderContext;	
}

namespace Gl {
	class ImageComposer;

	class VtPipeline : public Scene::Pipeline {
	public:
		// misc
		friend class Gfx;
		virtual const char* getName() const { return "vt"; };

		// making this the pipeline currently active/unactive
		virtual void bind( Scene::RenderContext* _context );
		virtual void unbind();

		// geometry passes
		// each pipeline needs geometry submitting, it should be
		// sent GeomPassCoutn times with start and end bracketing it
		virtual int getGeomPassCount();
		virtual void startGeomPass( int i );
		virtual void endGeomPass (int i );

		// merging or display results
		virtual void display( Scene::RenderContext* context, int backWidth, int backHeight );
		virtual void merge( Scene::RenderContext* rc );

		// setup 
		virtual void conditionWob( const char* name, struct Scene::WobResource* wob );

	protected:
		enum GEOM_PASSES {
			Z_PREPASS = 0,
			MAIN_PASS = 1,
		};
		VtPipeline( size_t index );
		virtual ~VtPipeline();

		const size_t pipelineIndex;

		// only valid between bind and unbind
		Gl::RenderContext* context;
		void bindZPrepassGeomPass();
		void bindMainGeomPass();

		Core::ScopedResourceHandle<TextureHandle>	depthRtHandle;
		Core::ScopedResourceHandle<TextureHandle>	colourRtHandle;

		Core::ScopedResourceHandle<ProgramHandle>	zprepassProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>	mainProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>	resolve8msaaProgramHandle;

		Core::ScopedResourceHandle<DataBufferHandle> dummyVBOHandle;
		Core::ScopedResourceHandle<VaoHandle> dummyVaoHandle;
	};

	class VtPipelineDataStore : public Scene::PipelineDataStore {
	public:
		friend class VtPipeline;
		~VtPipelineDataStore();

		void render( Scene::RenderContext* context );
	protected:
		struct PerMaterial {
			std::string					name;
			Vao::CreationStruct			vacs; // index and vertex buffer handles are inside
			VaoHandlePtr				vaoHandle;
			size_t						numIndices;
			int							indexType;
		};
		int numMaterials;
		boost::scoped_array<PerMaterial>	materials;
	};

}

#endif