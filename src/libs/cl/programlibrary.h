#pragma once
//!-----------------------------------------------------
//!
//! \file programlibrary.h
//! Contains the cl program library resource class
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_PROGRAM_LIBRARY_H)
#define WIERD_CL_PROGRAM_LIBRARY_H

#include "core/resources.h"

namespace Cl {
	class Context;

	static const uint32_t ProgramLibraryRType = RESOURCE_NAME('C','P','G','L');

	enum PROGRAMLIB_CREATION_FLAGS {
		PLCF_GPU_ONLY	= BIT(0),			// know this requires a GPU, so only compile for it
		PLCF_ANY_ONE	= BIT(1),			// as long as one device handles it, thats okay. default is for all devices
		PLCF_CPU_ONLY	= BIT(2),			// know this requires a CPU, so only compile for it
	};

	class ProgramLibrary :	public Core::Resource<ProgramLibraryRType> {
	public:	
		struct CreationStruct {
			Context*					context;	//<! which contex is the kernel for
			PROGRAMLIB_CREATION_FLAGS	flags;		//<! flags about whats okay and whats not
			DEVICE_EXTENSIONS_FLAGS		required;	//!< which extensions are required
		};
		struct LoadStruct {};

	protected:
		friend class ProgramMan;
		friend class Kernel;
		friend class Core::ResourceHandle<ProgramLibraryRType, ProgramLibrary>;

		cl_program	name;
	};

	typedef const Core::ResourceHandle<ProgramLibraryRType, ProgramLibrary> ProgramLibraryHandle;
	typedef ProgramLibraryHandle* ProgramLibraryHandlePtr;
	typedef std::shared_ptr<ProgramLibrary> ProgramLibraryPtr;

}
#endif // WIERD_CL_PROGRAM_LIBRARY_H