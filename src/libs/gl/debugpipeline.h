#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\debugpipeline.h
///
/// \brief	Declares the controlling class for debug rendering.
///
/// \remark	Copyright (c) 2012 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( YOLK_GL_DEBUG_PIPELINE_H__ )
#define YOLK_GL_DEBUG_PIPELINE_H__

#include "core/resources.h"
#include "scene/pipeline.h"
#include "vao.h"

namespace Scene {
	class RenderContext;	
}

namespace Gl {

	class DebugPipeline : public Scene::Pipeline {
	public:
		friend class Gfx;

		virtual const char* getName() const { return "debug"; };

		virtual void bind( Scene::RenderContext* _context );
		virtual void unbind( );

		virtual int getGeomPassCount() { return 1; }
		virtual void startGeomPass( int i ) {};
		virtual void endGeomPass (int i ) {};

		// no-op
		virtual void display( Scene::RenderContext* context, int backWidth, int backHeight );
		// not allowed
		virtual void merge( Scene::RenderContext* rc );

		//setup
		virtual void conditionWob( const char* name, struct Scene::WobResource* wob );
	protected:
		DebugPipeline( size_t index );
		virtual ~DebugPipeline();
		const size_t				pipelineIndex;

		Gl::RenderContext*			context;

		Core::ScopedResourceHandle<Scene::TextureHandle>			depthRtHandle;
		Core::ScopedResourceHandle<Scene::TextureHandle>			colourRtHandle;
		Core::ScopedResourceHandle<Scene::ProgramHandle>			programHandle;
	};

	class DebugPipelineDataStore : public Scene::PipelineDataStore {
	public:
		friend class DebugPipeline;
		~DebugPipelineDataStore();

		void render( Scene::RenderContext* context );
	protected:
		struct PerMaterial {
			std::string						name;
			Vao::CreationStruct				vacs; // vertex buffer handles are inside
			Scene::VertexInputHandlePtr		vaoHandle;
			size_t							numIndices;
			int								indexType;
			Scene::DataBufferHandlePtr		indexBuffer;
		};
		int numMaterials;
		boost::scoped_array<PerMaterial>	materials;
	};

}

#endif