//!-----------------------------------------------------
//!
//! \file skeleton.h
//! the engine graphics skeleton class
//!
//!-----------------------------------------------------
#if !defined(YOLK_SCENE_SKELETON_H_)
#define YOLK_SCENE_SKELETON_H_
#pragma once

namespace Scene {

	class RenderContext;

	class Skeleton {
	public:

		//! dtor
		virtual ~Skeleton(){};

		//! A pre render joint update
		virtual void updateJoints() = 0;
		virtual void debugDisplay( RenderContext* context ){};

	protected:
	};
}; // end namespace Graphics

#endif