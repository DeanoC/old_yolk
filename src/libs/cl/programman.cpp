/**
 @file	Z:\Projects\wierd\source\cl\programman.cpp

 @brief	Implements the program manager.
 */
#include "cl.h"
#include "core/resources.h"
#include "context.h"
#include "kernel.h"
#include "programlibrary.h"
#include "programman.h"

#define DECLARE_LIBRARY( x )		extern const char* binary_data_kernels_ ## x ## _cl;
#define REGISTER_LIBRARY( x )		programSrc[ #x ] = binary_data_kernels_ ## x ## _cl;

// outside name space
DECLARE_LIBRARY( helloworld );

namespace Cl {
ProgramMan::ProgramMan() {
	REGISTER_LIBRARY( helloworld );
}

#undef REGISTER_LIBRARY
#undef DECLARE_LIBRARY

const char* ProgramMan::getProgramLibrarySource( const std::string& prgName ) const {
	NameSourceMap::const_iterator cIt = programSrc.find( prgName );
	if( cIt == programSrc.end() ) {
		// no source under that name :(
		LOG(FATAL) << "Cl Program source " << prgName << " not found";
		return NULL;
	}
	return cIt->second;
}

void ProgramMan::registerLibrarySource( const char* name, const char* src ) {
	CORE_ASSERT( programSrc.find( name ) == programSrc.end() );
	programSrc[ name ] = src;
}

/// this get called if this program is not in the (in-memory) resource cache
ProgramLibrary* ProgramMan::internalCreate( const Core::ResourceHandleBase* baseHandle, const char* pName, const ProgramLibrary::CreationStruct* creation ) {
	using namespace Core;
	// TODO: baseHandle is used for async creation, not used currently

	// find program stage via, stage is _Xs_ or it at start just Xs_ where X is one
	// of v, f, g, tc, te 
	std::string programName( pName );

	const char* src = getProgramLibrarySource( programName );
	if( src == NULL )
		return NULL;

	ProgramLibrary* prg = CORE_NEW ProgramLibrary();
	std::vector<std::string> srcFragments;

	std::string srcString( src );
	preprocessIncludes( srcString, srcFragments );
	preprocessSharedSource( srcString );
	srcString += "\n";

	static const int MAX_CL_INCLUDES = 128;
	std::string clstrsrc[ MAX_CL_INCLUDES ];
	const char* clsrc[ MAX_CL_INCLUDES ];
	int count = 0;

/*	// preheader if required
	const Core::string ver420( "#version 420 core\n" );
	clsrc[count++] = ver420.c_str();
	IF_DEBUG( 
		const Core::string pragDebug( "#pragma debug(on)\n" );
		clsrc[count++] = pragDebug.c_str();
	);
	*/

	for( auto i = srcFragments.begin(); i != srcFragments.end(); ++i ) {
		const char* incSrc = getProgramLibrarySource(*i);
		clstrsrc[count] = incSrc;
		clstrsrc[count] += std::string("\n");
		preprocessIncludes( clstrsrc[count], srcFragments );
		preprocessSharedSource( clstrsrc[count] );
		clsrc[count] = clstrsrc[count].c_str();
		count++;
		CORE_ASSERT( count < MAX_CL_INCLUDES );
	}
	clsrc[count++] = srcString.c_str();
	CORE_ASSERT( count < MAX_CL_INCLUDES );

	Context* ctx = creation->context;
	cl_int _err;
	prg->name = clCreateProgramWithSource( ctx->getContext(), count, clsrc, nullptr, &_err );
	CL_CHECK( "clCreateProgramWithSource", _err);
	const char* compileOptions = 
#if DEBUG_LEVEL >= DEBUG_NONE
		"-cl-std=CL1.1 -cl-denorms-are-zero -cl-fast-relaxed-math -cl-strict-aliasing -cl-opt-disable -D DEBUG";
#else
		"-cl-std=CL1.1 -cl-denorms-are-zero -cl-fast-relaxed-math -cl-strict-aliasing";
#endif

	int numMatchingDevices = 0;
	cl_device_id * matchingDevices = (cl_device_id*) CORE_STACK_ALLOC( sizeof(cl_device_id) * ctx->getNumDevices() );
	if( creation->required == DEF_NONE && ((creation->flags & PLCF_GPU_ONLY) == 0) ) {
		numMatchingDevices = ctx->getNumDevices();
		matchingDevices = (cl_device_id*) ctx->getDevices();
	} else {
		for( int i = 0; i < ctx->getNumDevices(); ++i ) {
			if( (creation->required & ctx->getDeviceFlags()[i]) == creation->required ) {
				if( ((creation->flags & PLCF_GPU_ONLY) == 0 || ctx->isDeviceGpu(i) == true) || 
					((creation->flags & PLCF_CPU_ONLY) == 0 || ctx->isDeviceGpu(i) == false) ) {
					matchingDevices[numMatchingDevices++] = ctx->getDevices()[i];
				}
			}
		}
	}

	if( numMatchingDevices == 0 ) {
		LOG(FATAL) << "Open CL no devices can build " << programName;
		return nullptr;
	}
	_err = clBuildProgram( prg->name, numMatchingDevices, matchingDevices, compileOptions, nullptr, nullptr );
	if( _err != CL_SUCCESS ) {
		for( int i = 0; i < ctx->getNumDevices(); ++i ) {
			size_t logLength = 0;
			clGetProgramBuildInfo(prg->name, ctx->getDevices()[i], CL_PROGRAM_BUILD_LOG, 0, nullptr, &logLength);
			char* log = CORE_STACK_NEW_ARRAY( char, logLength );
			clGetProgramBuildInfo(prg->name, ctx->getDevices()[i], CL_PROGRAM_BUILD_LOG, logLength, log, nullptr);
			LOG(FATAL) << "Validate program :  " << programName << " : " << log;
		}
		return nullptr;
	} else {
		// 'binary' on both AMD + NV is actually IR (PTX or CAL), so can be handy to dump it to screen
/*
		size_t* binLengths = CORE_STACK_NEW_ARRAY( size_t, ctx->getNumDevices() );;
		clGetProgramInfo(prg->name, CL_PROGRAM_BINARY_SIZES, ctx->getNumDevices() * sizeof(size_t), binLengths, nullptr);
		char** logs = CORE_STACK_NEW_ARRAY( char*, ctx->getNumDevices() );
		for( int i = 0;i< ctx->getNumDevices(); ++i ) {
			logs[i] = CORE_STACK_NEW_ARRAY( char, binLengths[i] );
		}
		clGetProgramInfo(prg->name, CL_PROGRAM_BINARIES, ctx->getNumDevices() * sizeof(size_t), logs, nullptr);
		for( int i = 0;i< ctx->getNumDevices(); ++i ) {
			LOG(INFO) << programName << " Binary("<< i << ") :\n" << logs[i];
		}*/
	}
	size_t* binSizes = (size_t*) CORE_STACK_ALLOC(ctx->getNumDevices() * sizeof(size_t) );

	_err = clGetProgramInfo( prg->name, CL_PROGRAM_BINARY_SIZES, ctx->getNumDevices() * sizeof(size_t), binSizes, nullptr );
	CL_CHECK( "clGetProgramInfo", _err);

	int supportCount = 0;
	for( int i = 0; i < ctx->getNumDevices(); ++i ) {
		if(binSizes[i] == 0 ) {
			LOG(INFO) << "Open CL program library " << programName << " not available for device " << i;
		} else {
			supportCount++;
		}
	}

	// allow through only some devices if any_one or gpu_only or cpu_only
	if( (creation->flags & (PLCF_ANY_ONE | PLCF_GPU_ONLY | PLCF_CPU_ONLY)) == 0 ) {
		if( supportCount != ctx->getNumDevices() ) {
			LOG(FATAL) << "OpenCL program " << programName << " was requested for all devices, but not all can support it";
			return nullptr;
		}
	}

	if( supportCount == 0 ) {
		LOG(FATAL) << "Open CL no devices can execute " << programName;
		return nullptr;
	}

	return prg;
}

void ProgramMan::preprocessIncludes( std::string& src, std::vector<std::string>& fragments ) {
	using namespace Core;
	std::string::size_type index = 0;
	while( (index = src.find( "#include" )) != std::string::npos ) {
		src[index+0] = '/';
		src[index+1] = '/';
		std::string::size_type spmark0 = src.find( '"', index );
		std::string::size_type spmark1 = src.find( '.', spmark0+1 );
		CORE_ASSERT( spmark0 != std::string::npos );
		CORE_ASSERT( spmark1 != std::string::npos );
		CORE_ASSERT( spmark0 != spmark1 );
		std::string fragName( &src[spmark0+1], &src[spmark1] );
		std::replace( fragName.begin(), fragName.end(), '/', '_' );
		std::replace( fragName.begin(), fragName.end(), '\\', '_' );
		fragments.push_back( fragName );
	}
}

// to share some source between c++ we have to encode certain things (e.g #version)
// we encode by placing // comment and then making them whitespace here
void ProgramMan::preprocessSharedSource( std::string& src ) {
	using namespace Core;
	std::string::size_type index = 0;
	while( (index = src.find( "//#version" )) != std::string::npos ) {
		src[index+0] = ' ';
		src[index+1] = ' ';
	}
	index = 0;
	while( (index = src.find( "//#extension" )) != std::string::npos ) {
		src[index+0] = ' ';
		src[index+1] = ' ';
	}
}


}