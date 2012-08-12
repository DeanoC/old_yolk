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

#if !defined( VT_GL_VT_PIPELINE_H_ )
#define VT_GL_VT_PIPELINE_H_

#include "core/resources.h"
#include "scene/pipeline.h"
#include "vao.h"
//#include "cl/image.h"
//#include "cl/kernel.h"
//#include "cl/cmdqueue.h"

namespace Cl {
	class Context;
}

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
			FRAG_COUNTER = 0,
			CAPTURE_FRAGMENTS,
			MAIN_PASS,
			NUM_GEOM_PASSES,
		};
		const size_t pipelineIndex;

		VtPipeline( size_t index );
		virtual ~VtPipeline();

		void createCaptureBuffers();

		// only valid between bind and unbind
		Gl::RenderContext* context;
		void startMainGeomPass();
		void startFragCountGeomPass();
		void endFragCountGeomPass();
		void startCaptureFragmentsGeomPass();

#define CSRHS(x) Core::ScopedResourceHandle< Scene:: x >
#define CSRH(x) Core::ScopedResourceHandle< x >

		CSRHS( DataBufferHandle )	fragmentsBufferHandle;
		CSRHS( DataBufferHandle )	scratchInitHandle;
		CSRHS( DataBufferHandle )	scratchBufHandle;
		CSRHS( DataBufferHandle )	facesBufferHandle;

//		CSRH( Cl::BufferHandle )	fragmentsClBufferHandle;
//		CSRH( Cl::BufferHandle )	facesClBufferHandle;

		CSRH( TextureHandle )		depthRtHandle;
		CSRH( TextureHandle )		colourRtHandle;
		CSRH( TextureHandle )		fragCountRtHandle;
		CSRH( TextureHandle )		fragmentsTexHandle;
		CSRH( TextureHandle )		scratchTexHandle;
		CSRH( TextureHandle )		fragHeaderRtHandle;
		CSRH( TextureHandle )		pixelTempRtHandle;
		CSRH( TextureHandle )		facesTexHandle;

		CSRHS( ProgramHandle )		fragCountProgramHandle;
		CSRHS( ProgramHandle )		fragHeaderProgramHandle;
		CSRHS( ProgramHandle )		captureFragmentsProgramHandle;
		CSRHS( ProgramHandle )		mainProgramHandle;
		CSRHS( ProgramHandle )		resolve8msaaProgramHandle;
		CSRHS( ProgramHandle )		debugCaptureFragmentsProgramHandle;

		CSRHS( DataBufferHandle ) 	dummyVBOHandle;
		CSRHS( VertexInputHandle ) 	dummyVaoHandle;

#undef CSRSH
#undef CSRH

		unsigned int 				targetWidth;
		unsigned int 				targetHeight;
		unsigned int 				targetSamples;
		Cl::Context*				contextCl;

	};

	class VtPipelineDataStore : public Scene::PipelineDataStore {
	public:
		friend class VtPipeline;
		~VtPipelineDataStore();

		void render( Scene::RenderContext* context );
	protected:
		struct PerMaterial {
			std::string						name;
			Vao::CreationStruct				vacs; // index and vertex buffer handles are inside
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