//!-----------------------------------------------------
//!
//! \file kerne.cpp
//! the opencl kernel implementation
//!
//!-----------------------------------------------------

#include "cl.h"

#include "context.h"
#include "platform.h"
#include "programlibrary.h"

#include "kernel.h"

namespace Cl {

Kernel::~Kernel() {
	progLibHandle->close();
}

Kernel* Kernel::internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationStruct* creation ) {
	using namespace Core;

	const std::string progLibAndKernelName( pName );
	const std::string::size_type kernalNameStart = progLibAndKernelName.find( "|" );
	if( kernalNameStart == std::string::npos ) {
		LOG(ERROR) << progLibAndKernelName << " doesn't have a | separating library from kernel function name";
		return NULL;
	}
	const std::string progLibName( progLibAndKernelName.begin(), progLibAndKernelName.begin() + kernalNameStart );
	const std::string kernelName( progLibAndKernelName.begin() + kernalNameStart + 1, progLibAndKernelName.end() );

	// acquire the library our kernel is from
	ProgramLibrary::CreationStruct plcs = {
		creation->context,
		(PROGRAMLIB_CREATION_FLAGS) creation->flags,
		creation->required
	};
	Kernel* kernel = CORE_NEW Kernel();

	kernel->progLibHandle = ProgramLibraryHandle::create( progLibName.c_str(), &plcs );
	ProgramLibraryPtr progLib = kernel->progLibHandle->acquire();

	cl_int _err;
	kernel->name = clCreateKernel( progLib->name, kernelName.c_str(), &_err );
	CL_CHECK( "clCreateKernel", _err);

	if( kernel->name != 0 ) {
		return kernel;
	} else {
		LOG(FATAL) << "Cl Kernel " << kernelName << " cannot be created from program library " << progLibName;
		CORE_DELETE( kernel );
		return nullptr;
	}
}

}