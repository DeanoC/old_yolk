/**
 @file	Z:\Projects\wierd\source\gl\heightfield.h

 @brief	Declares the heightfield class.
 */
#pragma once
#if !defined( YOLK_GL_HEIGHTFIELD_H_ )
#define YOLK_GL_HEIGHTFIELD_H_

#include "memory.h"
#include "core/resources.h"
#include "core/resourceman.h"

namespace Gl {
	//! Heightfield Type
	static const uint32_t HeightfieldType = RESOURCE_NAME('H','F','D','1');

	class Heightfield : public Memory,
						public Core::Resource<HeightfieldType> {
	public:
	};

	typedef const Core::ResourceHandle<HeightfieldType, Heightfield> HeightfieldHandle;
	typedef HeightfieldHandle* HeightfieldHandlePtr;
	typedef std::shared_ptr<Heightfield> HeightfieldPtr;

}