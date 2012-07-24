#pragma once

#if !defined(YOLK_GL_STDAFX_H)
#define YOLK_GL_STDAFX_H

#include "core/core.h"
#include "core/vector_math.h"

#include "glew.h"

#include "glformat_cracker.h"

#define DECLARE_SHARED_WITH_CL( x ) 	extern const char* binary_data_programs_ ## x ## _glsl;
#define DECLARE_PROGRAM( x) 			extern const char* binary_data_programs_programs_ ## x ## _glsl; 
#define DECLARE_FRAGMENT( x )			extern const char* binary_data_programs_fragments_ ## x ## _glsl; 
#define DECLARE_CL_LIBRARY( x )			extern const char* binary_data_programs_kernels_ ## x ## _cl; 

// special GLSL definations, that can be shared with CL programs (lots of restrictions basically structures only!)
#define REGISTER_SHARED_WITH_CL(x)		Gfx::get()->getShaderMan()->registerProgramSource( #x, ::binary_data_programs_ ## x ## _glsl ); \
										Cl::Platform::get()->getProgramMan()->registerLibrarySource( #x , binary_data_programs_ ## x ## _glsl );
// GLSL programs or fragments
#define REGISTER_PROGRAM( x )			Gfx::get()->getShaderMan()->registerProgramSource( #x, binary_data_programs_programs_ ## x ## _glsl );
#define REGISTER_FRAGMENT( x )			Gfx::get()->getShaderMan()->registerProgramSource( "fragments_" #x, binary_data_programs_fragments_ ## x ## _glsl );
// CL librarys with kernels in them
#define REGISTER_CL_LIBRARY( x )		Cl::Platform::get()->getProgramMan()->registerLibrarySource( #x , binary_data_programs_kernels_ ## x ## _cl );

#define GDEBBUGGER_FRIENDLY_STARTUP


#define DO_GL_CHECKS

#if defined( DO_GL_CHECKS )
#define GL_LOGERROR( _cherr ) \
	switch( _cherr ) { \
	case GL_INVALID_ENUM: LOG(INFO) << "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument."; break; \
	case GL_INVALID_VALUE: LOG(INFO) << "GL_INVALID_VALUE: A numeric argument is out of range.."; break; \
	case GL_INVALID_OPERATION: LOG(INFO) << "GL_INVALID_OPERATION: The specified operation is not allowed in the current state."; break; \
	case GL_OUT_OF_MEMORY: LOG(INFO) << "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command."; break; \
	case GL_TABLE_TOO_LARGE: LOG(INFO) << "GL_TABLE_TOO_LARGE: The specified table exceeds the implementation's maximum supported table size."; break; \
	default: LOG(INFO) << "Unknown GL Error (" << _cherr << ")"; break; \
	}

#define GL_CHECK {															\
			GLenum _cherr = GL_NO_ERROR;									\
			bool   _cheer = true;											\
			while( (_cherr = glGetError()) != GL_NO_ERROR ) {				\
				GL_LOGERROR( _cherr );											\
				_cheer = false;												\
			}																\
			if( _cheer == false ) {											\
				LOG(FATAL) << "\nGL_CHECK failed\n";				\
			}																\
		}

#else 
#define GL_CHECK
#endif

// compute renderer needs CL
#include "cl/cl.h"


#endif // WIERD_GL_STDAFX_H
