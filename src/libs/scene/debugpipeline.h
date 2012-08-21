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
#include "scene/vertexinput.h"

namespace Scene {
	class RenderContext;	
}

namespace Scene {

	class DebugPipeline : public Pipeline {
	public:
		DebugPipeline();
		virtual ~DebugPipeline();

		virtual const char* getName() const override { return "debug"; };

		virtual void bind( RenderContext* _context ) override;
		virtual void unbind( RenderContext* _context ) override;

		virtual int getGeomPassCount() override { return 1; }
		virtual void startGeomPass( RenderContext* _context, int i ) override {};
		virtual void endGeomPass ( RenderContext* _context, int i ) override {};

		virtual TextureHandlePtr getResult() { return colourTargetHandle.get(); }

		//setup
		virtual void conditionWob( class Scene::Wob* wob ) override;
	protected:
		Core::ScopedResourceHandle<TextureHandle>			depthTargetHandle;
		Core::ScopedResourceHandle<TextureHandle>			colourTargetHandle;
		Core::ScopedResourceHandle<ProgramHandle>			programHandle;
		Core::ScopedResourceHandle<RasteriserStateHandle>	rasterStateHandle;
	};

	class DebugPipelineDataStore : public PipelineDataStore {
	public:
		friend class DebugPipeline;
		~DebugPipelineDataStore();

		void render( Scene::RenderContext* context );
	protected:
		struct PerMaterial {
			std::string						name;
			VertexInput::CreationInfo		vacs;
			VertexInputHandlePtr			vaoHandle;
			size_t							numIndices;
			int								indexSize;		// size 2 or 4
			Scene::DataBufferHandlePtr		indexBuffer;
		};
		int numMaterials;
		boost::scoped_array<PerMaterial>	materials;
	};

}

#endif