#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	render\pipeline.h
///
/// \brief	Declares the controlling signature for pipelines
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( YOLK_SCENE_PIPELINE_H_ )
#define YOLK_SCENE_PIPELINE_H_

#ifndef YOLK_SCENE_TEXTURE_H_
#include "texture.h"
#endif

namespace Scene {
	// forward decl
	class RenderContext;
	class Wob;

	class Pipeline {
	public:
		friend class Renderer;
		// misc
		virtual ~Pipeline(){};
		virtual const char* getName() const = 0;
		const size_t getIndex() const { return pipelineIndex; }

		// making this the pipeline currently active/unactive
		virtual void bind( RenderContext* _context ) = 0;
		virtual void unbind( RenderContext* _context ) = 0;

		// geometry passes
		// each pipeline needs geometry submitting, it should be
		// sent GeomPassCoutn times with start and end bracketing it
		virtual int getGeomPassCount() = 0;
		virtual bool isGeomPassOpaque( int pass ) = 0;
		virtual void startGeomPass( RenderContext* _context, int i ) = 0;
		virtual void endGeomPass ( RenderContext* _context, int i ) = 0;

		// where the pipeline work ends up
		virtual TextureHandlePtr getResult() = 0;
		// setup 
		virtual void conditionWob( Wob* wob ) = 0;
	protected:
		void setRendererIndex( size_t index ) { pipelineIndex = index; }
		Pipeline () : pipelineIndex( ~0 ){};
		size_t				pipelineIndex;

	};

	// pipeline data should derive off this, which will ensure it get 
	// correctly deleted
	class PipelineDataStore {
	public:
		virtual ~PipelineDataStore(){};
		virtual void render( Scene::RenderContext* context ) = 0;
		virtual void renderTransparent( Scene::RenderContext* context ) = 0;
	};
}

#endif