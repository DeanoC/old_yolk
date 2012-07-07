#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\forwardpipeline.h
///
/// \brief	Declares the controlling class for forward rendering.
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( WIERD_GL_FORWARD_PIPELINE_H__ )
#define WIERD_GL_FORWARD_PIPELINE_H__

#include "core/resources.h"
#include "scene/pipeline.h"
#include "vao.h"

namespace Scene {
	class RenderContext;	
}

namespace Gl {

	class ForwardPipeline : public Scene::Pipeline {
	public:
		friend class Gfx;

		virtual const char* getName() const { return "forward"; };

		virtual void bind( Scene::RenderContext* _context, bool clear );
		virtual void unbind( Scene::RenderContext* context );

		virtual void display( Scene::RenderContext* context, int backWidth, int backHeight );

		virtual void conditionWob( const char* name, struct Scene::WobResource* wob );

	protected:
		ForwardPipeline( size_t index );
		virtual ~ForwardPipeline();

		Core::ScopedResourceHandle<TextureHandle>	depthRtHandle;
		Core::ScopedResourceHandle<TextureHandle>	colourRtHandle;
		Core::ScopedResourceHandle<ProgramHandle>	programHandle;
		size_t				pipelineIndex;
	};

	class ForwardPipelineDataStore : public Scene::PipelineDataStore {
	public:
		friend class ForwardPipeline;
		~ForwardPipelineDataStore();

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