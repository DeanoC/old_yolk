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

namespace Scene {
	// forward decl
	class RenderContext;

	class Pipeline {
	public:
		// misc
		virtual ~Pipeline(){};
		virtual const char* getName() const = 0;

		// making this the pipeline currently active/unactive
		virtual void bind( RenderContext* _context ) = 0;
		virtual void unbind() = 0;

		// geometry passes
		// each pipeline needs geometry submitting, it should be
		// sent GeomPassCoutn times with start and end bracketing it
		virtual int getGeomPassCount() = 0;
		virtual void startGeomPass( int i ) = 0;
		virtual void endGeomPass ( int i ) = 0;

		// merging or display results
		virtual void display( RenderContext* context, int backWidth, int backHeight ) = 0;
		virtual void merge( RenderContext* rc ) = 0;

		// setup 
		virtual void conditionWob( const char* name, struct WobResource* wob ) = 0;
	protected:
	};

	// pipeline data should derive off this, which will ensure it get 
	// correctly deleted
	class PipelineDataStore {
	public:
		virtual ~PipelineDataStore(){};
		virtual void render( Scene::RenderContext* context ) = 0;
	};
}

#endif