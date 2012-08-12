#pragma once

//!-----------------------------------------------------
//!
//! \file program.h
//! A Gl Program is a resource representing a compiled
//! GPU shader
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_PROGRAM_H_ )
#define YOLK_SCENE_PROGRAM_H_

#include <core/resources.h>
#include "memory.h"

namespace Scene {
	static const uint32_t ProgramRType = RESOURCE_NAME('G','P','R','G');

	class Program : public Core::Resource<ProgramRType> {
	public:
		struct CreationStruct {
			int				numTransformFeedbackItems;
			bool			interleavedItems;
			const char*		transformFeedbackItemNames[ 16 ]; // max items
		};
		struct LoadStruct {};

		// bit flags of CONSTANT_FREQ tell which buffer are used by this program
		uint32_t getUsedBuffers() const { return usedBuffers; }

	protected:
		Program(){};
		uint32_t			usedBuffers;
	};

	typedef const Core::ResourceHandle<ProgramRType, Program> ProgramHandle;
	typedef ProgramHandle* ProgramHandlePtr;
	typedef std::shared_ptr<Program> ProgramPtr;

}

#endif