#pragma once
//!-----------------------------------------------------
//!
//! \file kernel.h
//! Contains the cl kernel resource class
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_KERNEL_H)
#define WIERD_CL_KERNEL_H

#include "core/resources.h"
#include "image.h"
#include "buffer.h"
#include "programlibrary.h"

namespace Cl {
	class Context;

	static const uint32_t KernelRType = RESOURCE_NAME('C','K','N','L');

	enum KERNEL_CREATION_FLAGS {
		KCF_GPU_ONLY	= PLCF_GPU_ONLY,			// know this requires a GPU, so only compile for it
		KCF_ANY_ONE		= PLCF_ANY_ONE,			// as long as one device handles it, thats okay. default is for all devices
		KCF_CPU_ONLY	= PLCF_CPU_ONLY,			// know this requires a CPU, so only compile for it
	};

	namespace Private {
		template< typename T> inline void setArg( cl_kernel name, unsigned int num, const T& arg );

		template<> inline void setArg<ImageHandle>( cl_kernel name, unsigned int num, const ImageHandle& argHandle ) {
			auto arg = argHandle.acquire();
			MemoryObject::Name imageName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &imageName );
			CL_CHECK( "clSetKernelArg", _err );
		}
		template<> inline void setArg<BufferHandle>( cl_kernel name, unsigned int num, const BufferHandle& argHandle ) {
			auto arg = argHandle.acquire();
			MemoryObject::Name bufferName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &bufferName );
			CL_CHECK( "clSetKernelArg", _err );
		}
		template<> inline void setArg<ImageHandlePtr>( cl_kernel name, unsigned int num, const ImageHandlePtr& argHandle ) {
			auto arg = argHandle->acquire();
			MemoryObject::Name imageName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &imageName );
			CL_CHECK( "clSetKernelArg", _err );
		}
		template<> inline void setArg<BufferHandlePtr>( cl_kernel name, unsigned int num, const BufferHandlePtr& argHandle ) {
			auto arg = argHandle->acquire();
			MemoryObject::Name bufferName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &bufferName );
			CL_CHECK( "clSetKernelArg", _err );
		}
		template<> inline void setArg<Core::ScopedResourceHandle<ImageHandle>>( cl_kernel name, unsigned int num, const Core::ScopedResourceHandle<ImageHandle>& argHandle ) {
			auto arg = argHandle.acquire();
			MemoryObject::Name imageName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &imageName );
			CL_CHECK( "clSetKernelArg", _err );
		}
		template<> inline void setArg<Core::ScopedResourceHandle<BufferHandle>>( cl_kernel name, unsigned int num, const Core::ScopedResourceHandle<BufferHandle>& argHandle ) {
			auto arg = argHandle.acquire();
			MemoryObject::Name bufferName = arg->getName();
			cl_int _err = clSetKernelArg( name, num, sizeof(MemoryObject::Name), &bufferName );
			CL_CHECK( "clSetKernelArg", _err );
		}

	}

	class Kernel :	public Core::Resource<KernelRType> {
	public:	
		~Kernel();
		struct CreationInfo {
			Context*					context;	//<! which contex is the kernel for
			KERNEL_CREATION_FLAGS		flags;		
			DEVICE_EXTENSIONS_FLAGS		required;	//!< which extensions are required
		};
		static Kernel* internalCreate( const Core::ResourceHandleBase* handle, const char* pName, const CreationInfo* creation );

		cl_kernel			getName() const { return name; }
		template< typename T> void setArg( unsigned int num, const T& arg ) {
			Private::setArg( name, num, arg );
		}


		// allocate a local buffer for the kernel of size, pointer will be passed to kernel in arg num
		void setLocalBufferArg( unsigned int num, size_t size ) {
			cl_int _err = clSetKernelArg( name, num, size, NULL );
			CL_CHECK( "clSetKernelArg", _err );
		}

	protected:
		ProgramLibraryHandlePtr progLibHandle;
		cl_kernel			name;
	};

	typedef const Core::ResourceHandle<KernelRType, Kernel> KernelHandle;
	typedef KernelHandle* KernelHandlePtr;
	typedef std::shared_ptr<Kernel> KernelPtr;

}
#endif // WIERD_CL_KERNEL_H