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
#include "scene/gpu_constants.h"

namespace Scene {
	class RenderContext;	

	class VtPipeline : public Pipeline {
	public:
		VtPipeline();
		virtual ~VtPipeline();

		virtual const char* getName() const override { return "VT"; };

		virtual void bind( RenderContext* _context ) override;
		virtual void unbind( RenderContext* _context ) override;

		virtual int getGeomPassCount() override;
		virtual bool isGeomPassOpaque( int pass ) override;
		virtual void startGeomPass( RenderContext* _context, int pass ) override;
		virtual void endGeomPass ( RenderContext* _context, int pass ) override;

		virtual TextureHandlePtr getResult() { return colourTargetHandle.get(); }

		//setup
		virtual void conditionWob( class Scene::Wob* wob ) override;
	protected:
		void startGeomRenderOpaquePass( RenderContext* _context );
		void startGeomCountTransparentPass( RenderContext* _context );
		void startGeomRenderTransparentPass( RenderContext* _context );
		void endGeomRenderOpaquePass( RenderContext* _context );
		void endGeomCountTransparentPass( RenderContext* _context );
		void endGeomRenderTransparentPass( RenderContext* _context );
		Core::ScopedResourceHandle<TextureHandle>			colourTargetHandle;

		Core::ScopedResourceHandle<TextureHandle>			depthTargetMSHandle;
		Core::ScopedResourceHandle<TextureHandle>			gBuffer0MSHandle;
		Core::ScopedResourceHandle<TextureHandle>			gBuffer1MSHandle;
		Core::ScopedResourceHandle<TextureHandle>			tfcMSHandle;

		Core::ScopedResourceHandle<ProgramHandle>			solidWireFrameProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>			resolveProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>			lightingProgramHandle;
		Core::ScopedResourceHandle<ProgramHandle>			transCountProgramHandle;

		Core::ScopedResourceHandle<RasteriserStateHandle>	rasterStateHandle;
		Core::ScopedResourceHandle<RasteriserStateHandle>	rasterStateNoMSHandle;
		Core::ScopedResourceHandle<DepthStencilStateHandle>	depthStencilStateHandle;
		Core::ScopedResourceHandle<DepthStencilStateHandle>	depthStencilNoWriteStateHandle;
		Core::ScopedResourceHandle<RenderTargetStatesHandle> renderTargetWriteHandle;
		Core::ScopedResourceHandle<RenderTargetStatesHandle> renderTargetAddWriteHandle;

		bool												gpuMaterialStoreOk;
		Core::ScopedResourceHandle<DataBufferHandle>		materialStoreHandle;
		std::vector<GPUConstants::VtMaterial>				materialStoreSystemMem;

		bool												gpuLightStoreOk;
		Core::ScopedResourceHandle<DataBufferHandle>		lightStoreHandle;
		std::vector<GPUConstants::VtDirectionalLight>		directionalLightStoreSystemMem;

	};

	class VtPipelineDataStore : public PipelineDataStore {
	public:
		friend class VtPipeline;
		~VtPipelineDataStore();

		void render( Scene::RenderContext* context );
		void renderTransparent( Scene::RenderContext* context );
	protected:
		struct PerMaterial {
			std::string						name;
			VertexInput::CreationInfo		vacs;
			VertexInputHandlePtr			vaoHandle;
			size_t							numIndices;
			int								indexSize;		// size 2 or 4
			Scene::DataBufferHandlePtr		indexBuffer;
			uint32_t						materialIndex;
			bool							isTransparent;
		};
		int numMaterials;
		boost::scoped_array<PerMaterial>	materials;
	};

}

#endif