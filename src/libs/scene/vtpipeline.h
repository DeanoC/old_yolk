#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	scene\vtpipeline.h
///
/// \brief	Declares the controlling class for debug rendering.
///
/// \remark	Copyright (c) 2012 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( YOLK_SCENE_VT_PIPELINE_H_ )
#define YOLK_SCENE_VT_PIPELINE_H_

#include "core/resources.h"
#include "scene/pipeline.h"
#include "scene/vertexinput.h"

namespace Scene {
	class RenderContext;	
}

namespace Scene {

	class VtPipeline : public Pipeline {
	public:
		VtPipeline();
		virtual ~VtPipeline();

		virtual const char* getName() const override { return "VT"; };

		virtual void bind( RenderContext* _context ) override;
		virtual void unbind( RenderContext* _context ) override;

		virtual int getGeomPassCount() override { return 1; }
		virtual void startGeomPass( RenderContext* _context, int i ) override {};
		virtual void endGeomPass ( RenderContext* _context, int i ) override {};

		virtual TextureHandlePtr getResult() { return colourTargetHandle.get(); }

		//setup
		virtual void conditionWob( class Scene::Wob* wob ) override;
	protected:
		Core::ScopedResourceHandle<TextureHandle>			colourTargetHandle;
		Core::ScopedResourceHandle<TextureHandle>			colourTargetMSHandle;
		Core::ScopedResourceHandle<TextureHandle>			depthTargetMSHandle;

		Core::ScopedResourceHandle<ProgramHandle>			solidWireFrameProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>			resolveProgramHandle;
		Core::ScopedResourceHandle<RasteriserStateHandle>	rasterStateHandle;
		Core::ScopedResourceHandle<DepthStencilStateHandle>	depthStencilStateHandle;
		Core::ScopedResourceHandle<RenderTargetStatesHandle> renderTargetWriteHandle;
	};

	class VtPipelineDataStore : public PipelineDataStore {
	public:
		friend class VtPipeline;
		~VtPipelineDataStore();

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