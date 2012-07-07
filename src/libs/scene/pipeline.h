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
		virtual ~Pipeline(){};
		
		virtual const char* getName() const = 0;

		virtual void bind( RenderContext* _context, bool clear ) = 0;
		virtual void unbind( RenderContext* context ) = 0;

		virtual void display( RenderContext* context, int backWidth, int backHeight ) = 0;

		virtual void conditionWob( const char* name, struct WobResource* wob ) = 0;
	protected:
	};

	// pipeline data should derive off this, which will ensure it get 
	// correctly deleted
	class PipelineDataStore {
	public:
		virtual ~PipelineDataStore(){}
		virtual void render( RenderContext* context ) = 0;
	};
}

#endif