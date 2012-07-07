//!-----------------------------------------------------
//!
//! \file platform.cpp
//! the opencl platform singleton
//!
//!-----------------------------------------------------

#include "cl.h"

#if 1 //defined(USE_OPENGL)
#include "gl/gl.h"
#include "gl/gfx.h"
#include "gl/rendercontext.h"
#endif

#include "context.h"
#include "buffer.h"
#include "programlibrary.h"
#include "kernel.h"
#include "programman.h"
#include "platform.h"

#pragma comment( lib, "opencl" )

namespace {

void CL_CALLBACK ClNotify( const char *errinfo, const void* privateData, size_t cb, void* userData ) {
	LOG(INFO) << "OpenCL notify message : " << errinfo;
}
}

clGetGLContextInfoKHR_fn clGetGLContextInfoKHR;
clCreateEventFromGLsyncKHR_fn clCreateEventFromGLsyncKHR;

namespace Cl {

Platform::Platform() {

	installResourceTypes();

	cl_platform_id platforms[ MAX_PLATFORMS ];
	cl_uint numPlatforms = 0;
	clGetPlatformIDs( MAX_PLATFORMS, platforms, &numPlatforms );
	CORE_ASSERT( numPlatforms > 0 );

	int _err;
	totalDeviceCount = 0;

	for( cl_uint i = 0;i < numPlatforms; ++i ) {
		char buff[256];
		char profile[256];
		clGetPlatformInfo( platforms[i], CL_PLATFORM_PROFILE, 256, profile, nullptr );
		LOG(INFO) << "Profile : " << profile;
		clGetPlatformInfo( platforms[i], CL_PLATFORM_VERSION, 256, buff, nullptr );
		LOG(INFO) << "Version : " << buff;
		clGetPlatformInfo( platforms[i], CL_PLATFORM_NAME, 256, buff, nullptr );
		LOG(INFO) << "Name : " << buff;
		clGetPlatformInfo( platforms[i], CL_PLATFORM_VENDOR, 256, buff, nullptr );
		LOG(INFO) << "Vendor : " << buff;

		size_t sizeofExtString;
		clGetPlatformInfo( platforms[i], CL_PLATFORM_EXTENSIONS, 256, nullptr, &sizeofExtString );
		char* extString = (char*) CORE_STACK_ALLOC(sizeofExtString);
		clGetPlatformInfo( platforms[i], CL_PLATFORM_EXTENSIONS, sizeofExtString, extString, nullptr );
		LOG(INFO) << "Extensions : " << extString;

		cl_device_id platDevices[ MAX_DEVICES ];
		cl_uint		numPlatDevices = 0;

		clGetDeviceIDs( platforms[i], CL_DEVICE_TYPE_ALL, MAX_DEVICES, platDevices, &numPlatDevices );
		for( cl_uint j = 0;j < numPlatDevices; ++j, ++totalDeviceCount ) {
			cl_device_type type;
			cl_uint gblCacheType = 0;
			cl_uint gblCacheLine = 0;
			cl_ulong gblCacheSize = 0;
			cl_ulong gblMemSize = 0;
			cl_uint computeUnits = 0;
			cl_uint maxClockFreq = 0;
			cl_uint nativeFloatWidth = 0;
			cl_uint preferredFloatWidth = 0;
			cl_ulong constantMemSize = 0;
			cl_uint lclMemType = 0;
			cl_ulong lclMemSize = 0;
			cl_ulong execCap = 0;
			cl_ulong cmdBufCap = 0;
			cl_uint imageSupport = 0;
			size_t	image2DWidth = 0;
			size_t	image2DHeight = 0;
			clGetDeviceInfo( platDevices[j], CL_DEVICE_TYPE, sizeof(type), &type, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(computeUnits), &computeUnits, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(maxClockFreq), &maxClockFreq, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT, sizeof(nativeFloatWidth), &nativeFloatWidth, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT, sizeof(preferredFloatWidth), &preferredFloatWidth, nullptr );
			LOG(INFO) 
				<< ((type & CL_DEVICE_TYPE_CPU) ? "CPU " : "")
				<< ((type & CL_DEVICE_TYPE_GPU) ? "GPU " : "")
				<< " with " << computeUnits
				<< " compute units (float width = " << nativeFloatWidth << " preferred = " << preferredFloatWidth
				<< ") @ " << maxClockFreq;
			clGetDeviceInfo( platDevices[j], CL_DEVICE_GLOBAL_MEM_CACHE_TYPE, sizeof(gblCacheType), &gblCacheType, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(gblCacheLine), &gblCacheLine, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(gblCacheSize), &gblCacheSize, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(gblMemSize), &gblMemSize, nullptr );
			LOG(INFO)
				<< "Global Mem : " << gblMemSize/1024 << " KiB"
				<< "	 Cache : " << gblCacheSize/1024 << " KiB"
				<< "	  line : " << gblCacheLine << " B"
				<< "      type : " << ((gblCacheType == CL_READ_ONLY_CACHE) ? "Read only" : (gblCacheType == CL_READ_WRITE_CACHE) ? "Read/Write" : "None" );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(constantMemSize), &constantMemSize, nullptr );
			LOG(INFO)
				<< "Constant Mem : " << constantMemSize/1024 << " KiB";
			clGetDeviceInfo( platDevices[j], CL_DEVICE_LOCAL_MEM_TYPE, sizeof(lclMemType), &lclMemType, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(lclMemSize), &lclMemSize, nullptr );
			LOG(INFO)
				<< "Local Mem : " << lclMemSize/1024 << " KiB"
				<< "      type : " << ((lclMemType == CL_LOCAL) ? "Local" : "Global" );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_EXECUTION_CAPABILITIES, sizeof(execCap), &execCap, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_QUEUE_PROPERTIES, sizeof(cmdBufCap), &cmdBufCap, nullptr );
			clGetDeviceInfo( platDevices[j], CL_DEVICE_IMAGE_SUPPORT, sizeof(imageSupport), &imageSupport, nullptr );
			LOG(INFO)
				<< ((execCap & CL_EXEC_KERNEL ) ? "Exec OpenCL kernels" : "")
				<< ((execCap & CL_EXEC_NATIVE_KERNEL ) ? ", Exec Native kernels" : "")
				<< ((cmdBufCap & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE ) ? ", Out of Order Avail"  : "")
				<< ((cmdBufCap & CL_QUEUE_PROFILING_ENABLE ) ? ", Profiling"  : "") 
				<< (imageSupport ? ", Image Support" : "");
			if( imageSupport ) {
				clGetDeviceInfo( platDevices[j], CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(image2DWidth), &image2DWidth, nullptr );
				clGetDeviceInfo( platDevices[j], CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(image2DHeight), &image2DHeight, nullptr );
				LOG(INFO)
					<< "Image2D : Max Width = " << image2DWidth
					<< ", Max Height = " << image2DHeight;
			}


			size_t sizeofDevExtString = 0;
			_err = clGetDeviceInfo( platDevices[j], CL_DEVICE_EXTENSIONS, sizeofDevExtString, nullptr, &sizeofDevExtString );
			CL_CHECK( "clGetDeviceInfo CL_DEVICE_EXTENSIONS", _err );
			char* extDevCString = (char*) CORE_STACK_ALLOC(sizeofDevExtString);
			clGetDeviceInfo( platDevices[j], CL_DEVICE_EXTENSIONS, sizeofDevExtString, extDevCString, &sizeofDevExtString );
			LOG(INFO) << "Extensions : " << extDevCString;
			std::string extDevString( extDevCString );

			// add to the global device list
			devices[totalDeviceCount].id = platDevices[j];
			devices[totalDeviceCount].gpu = (type & CL_DEVICE_TYPE_GPU) != 0;
			devices[totalDeviceCount].glShare = (extDevString.find("cl_khr_gl_sharing") != extDevString.npos);
			devices[totalDeviceCount].rating = computeUnits * maxClockFreq * nativeFloatWidth * (devices[totalDeviceCount].gpu * 16);
			devices[totalDeviceCount].platformId = platforms[i];
			devices[totalDeviceCount].context = 0;
			devices[totalDeviceCount].flags = 0;
			devices[totalDeviceCount].flags |= (imageSupport > 0) ? DEF_IMAGES : 0;
			devices[totalDeviceCount].flags |= (std::string(profile).find("FULL_PROFILE") != std::string::npos ) ? DEF_FULL_PROFILE : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_amd_printf") != extDevString.npos) ? DEF_AMD_PRINTF : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_khr_global_int32_extended_atomics") != extDevString.npos) ? DEF_KHR_GLOBAL_EXT_ATOMIC_I32 : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_khr_local_int32_extended_atomics") != extDevString.npos) ? DEF_KHR_LOCAL_EXT_ATOMIC_I32 : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_khr_3d_image_writes") != extDevString.npos) ? DEF_KHR_3D_IMAGE_WRITES : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_ext_atomic_counters_32") != extDevString.npos) ? DEF_EXT_ATOMIC_COUNTERS_32 : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_amd_vec3") != extDevString.npos) ? DEF_AMD_VEC3 : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_amd_media_ops") != extDevString.npos) ? DEF_AMD_MEDIA_OPS : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_amd_popcnt") != extDevString.npos) ? DEF_AMD_POPCNT : 0;
			devices[totalDeviceCount].flags |= (extDevString.find("cl_ext_device_fission") != extDevString.npos) ? DEF_EXT_DEVICE_FISSION : 0;

		}
	}
	std::sort( devices.begin(), devices.begin() + totalDeviceCount, 
		[] ( const DeviceStruct& a, const DeviceStruct& b) -> bool { 
			int ratA = a.rating;
			int ratB = b.rating;
			// favor GPUs in sorted order and if we can share with gl
			ratA *= (a.glShare + 1) * 10;
			ratA *= (a.gpu + 1) * 5;
			ratB *= (b.glShare + 1) * 10;
			ratB *= (b.gpu + 1) * 5;

			return ratA > ratB;
		}
	);

	// add extentions functions pointers
    clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
    if (!clGetGLContextInfoKHR) {
        LOG(FATAL) << "Failed to query proc address for clGetGLContextInfoKHR";
    }
	/*
    clCreateEventFromGLsyncKHR = (clCreateEventFromGLsyncKHR_fn) clGetExtensionFunctionAddress("clCreateEventFromGLsyncKHR");
    if (!clCreateEventFromGLsyncKHR) {
        LOG(FATAL) << "Failed to query proc address for clCreateEventFromGLsyncKHR";
    }*/

	programMan.reset( CORE_NEW ProgramMan() );
}
void Platform::createDevices() {
	// no devices to create or already created, then nothing todo
	if( totalDeviceCount <= 0 ) return;
	if( devices[0].context != 0 ) return;

#if 1 //defined(USE_OPENGL)
	// first gpu device want to be connected to the Gl context, if we are using Gl
	// TODO multiple shared GPU devices (at the mo 1 shared GPU + N GPU/CPU compute only are possible)
	// TODO non Windows platforms
	if( Gl::Gfx::exists() ) {
		Gl::Gfx* gfx = Gl::Gfx::get();
		// device zero should be a glShare device (and most likely a GPU)
		// TODO handle if this isn't the case
		CORE_ASSERT( devices[0].glShare == true );
		cl_context_properties props[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties) devices[0].platformId, 
#if PLATFORM == WINDOWS
			CL_WGL_HDC_KHR, (cl_context_properties) gfx->getThreadRenderContext(0)->hDC, 
			CL_GL_CONTEXT_KHR, (cl_context_properties) gfx->getThreadRenderContext(0)->hRC,
#elif PLATFORM == POSIX 
			CL_GL_CONTEXT_KHR, (cl_context_properties) gfx->getThreadRenderContext( Gl::Gfx::RENDER_CONTEXT )->glxContext,
			CL_GLX_DISPLAY_KHR, (cl_context_properties) gfx->getThreadRenderContext( Gl::Gfx::RENDER_CONTEXT )->x11Display,
#endif
			0, 0
		};
		cl_device_id interopDevice;
		clGetGLContextInfoKHR( props, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), &interopDevice, nullptr );
		CORE_ASSERT( devices[0].id == interopDevice );

		cl_device_id platDevices[ MAX_DEVICES ] = { 0 };
		int platDeviceIndices[ MAX_DEVICES ] = { 0 };
		int platDeviceCount = 0;
		for( int i = 0; i < totalDeviceCount; ++ i ) {
			if( devices[i].platformId == devices[0].platformId ) {	
				platDevices[platDeviceCount] = devices[i].id;
				platDeviceIndices[platDeviceCount] = i;
				platDeviceCount++;
			}
		}
		cl_int _err;
		cl_context glContext = clCreateContext( props, platDeviceCount, platDevices, &ClNotify, nullptr, &_err);
		CL_CHECK( "clCreateContext", _err);
		for( int i = 0; i < totalDeviceCount; ++ i ) {
			if( devices[i].platformId = devices[0].platformId ) {	
				devices[i].context = glContext;
				clRetainContext( glContext ); // inc ref count so each device owns it equally
			}
		}
		primaryContext.reset( CORE_NEW Context(glContext, true ) );
		for( int i = 0; i < platDeviceCount; ++ i ) {
			primaryContext->addDevice( platDevices[i], (i == 0), (DEVICE_EXTENSIONS_FLAGS) devices[platDeviceIndices[i]].flags );
		}

		// the initial create, inc'ed the ref count so we need to release that now as each device has its own reference
		clReleaseContext( glContext );
	}
	// now GPU connected devices are sorted, the rest are just compute only devices
#endif
	// TODO computes are currently given there own contexts, could share when same platform...
	for( int i = 0; i < totalDeviceCount; ++ i ) {
		if( devices[i].context == 0 ) {
			cl_context_properties props[] = {
				CL_CONTEXT_PLATFORM, (cl_context_properties) devices[i].platformId, 
				0, 0
			};
			cl_int _err;
			cl_context computeContext = clCreateContext( props, 1, &devices[i].id, ClNotify, nullptr, &_err );
			CL_CHECK( "clCreateContext", _err);

			devices[i].context = computeContext;
			if( primaryContext == NULL ) {
				primaryContext.reset( CORE_NEW Context(computeContext, false) );
				primaryContext->addDevice( devices[i].id, false, (DEVICE_EXTENSIONS_FLAGS) devices[i].flags );
			} else {
				secondaryContexts.push_back( ContextPtr( CORE_NEW Context(computeContext, false )) );
				secondaryContexts.back()->addDevice( devices[i].id, false, (DEVICE_EXTENSIONS_FLAGS) devices[i].flags );
			}
		}
	}
}

void Platform::destroyDevices() {
	for( int i = 0; i < totalDeviceCount; ++ i ) {
		if( devices[i].context != 0 ) {
			clReleaseContext( devices[i].context );
			devices[i].context = 0;
		}
	}
}

Platform::~Platform() {
	destroyDevices();
}

//! Callback from the resource manager to create a data buffer
std::shared_ptr<Core::ResourceBase> BufferCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	const Buffer::CreationStruct* creation = (const Buffer::CreationStruct*) pData;

	// currently only in memory is supported
	BufferPtr pResource( Buffer::internalCreate( handle, pName, creation ) );
	return std::shared_ptr<ResourceBase>( pResource );
}

std::shared_ptr<Core::ResourceBase> ProgramLibraryCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	const ProgramLibrary::CreationStruct* creation = (const ProgramLibrary::CreationStruct*) pData;

	// currently only in memory is supported
	ProgramLibraryPtr pResource( Platform::get()->getProgramMan()->internalCreate( handle, pName, creation ) );
	return std::shared_ptr<ResourceBase>( pResource );
}

std::shared_ptr<Core::ResourceBase> KernelCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	const Kernel::CreationStruct* creation = (const Kernel::CreationStruct*) pData;

	// currently only in memory is supported
	KernelPtr pResource( Kernel::internalCreate( handle, pName, creation ) );
	return std::shared_ptr<ResourceBase>( pResource );
}

std::shared_ptr<Core::ResourceBase> ImageCreateResource( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Core;

	const Image::CreationStruct* creation = (const Image::CreationStruct*) pData;

	// currently only in memory is supported
	ImagePtr pResource( Image::internalCreate( handle, pName, creation ) );
	return std::shared_ptr<ResourceBase>( pResource );
}

void ClProcessLoader( const Core::ResourceHandleBase* handle, Core::RESOURCE_FLAGS flags, const char* pName, const void* pData  ) {
	using namespace Scene;

	std::shared_ptr<Core::ResourceBase> res;
	// route type to their specific creation functions 
	switch( handle->getType() ) {
	case BufferRType: res = BufferCreateResource( handle, flags, pName, pData ); break;
	case ProgramLibraryRType: res = ProgramLibraryCreateResource( handle, flags, pName, pData ); break;
	case KernelRType: res = KernelCreateResource( handle, flags, pName, pData );break;
	case ImageRType: res = ImageCreateResource( handle, flags, pName, pData ); break;
	default:
		LOG(FATAL) << "ClProcessLoader being passed a resource it cannot handle\n";
	}

	Core::ResourceMan::get()->internalAcquireComplete( handle, res );
}



void Platform::installResourceTypes() {
	using namespace Core;
	ResourceMan::get()->registerResourceType( BufferRType, ClProcessLoader, &SimpleResourceDestroyer<Buffer>, sizeof(BufferHandle), NULL, 0, "" );
	ResourceMan::get()->registerResourceType( ProgramLibraryRType, ClProcessLoader, &SimpleResourceDestroyer<ProgramLibrary>, sizeof(ProgramLibraryHandle), NULL, 0, "" );
	ResourceMan::get()->registerResourceType( KernelRType, ClProcessLoader, &SimpleResourceDestroyer<Kernel>, sizeof(KernelHandle), NULL, 0, "" );
	ResourceMan::get()->registerResourceType( ImageRType, ClProcessLoader, &SimpleResourceDestroyer<Image>, sizeof(ImageHandle), NULL, 0, "" );
}

} // End Cl Namespace

#define CLEC( x ) case x: errname = #x; break;
void clPrintErr(cl_int errcode, const char *file, const int line, const char *msg) {
	const char* errname = NULL;
	switch (errcode) {
	CLEC( CL_SUCCESS ) 
	CLEC( CL_DEVICE_NOT_FOUND )
	CLEC( CL_DEVICE_NOT_AVAILABLE )
	CLEC( CL_COMPILER_NOT_AVAILABLE )
	CLEC( CL_MEM_OBJECT_ALLOCATION_FAILURE )
	CLEC( CL_OUT_OF_RESOURCES              )
	CLEC( CL_OUT_OF_HOST_MEMORY            )
	CLEC( CL_PROFILING_INFO_NOT_AVAILABLE  )
	CLEC( CL_MEM_COPY_OVERLAP              )
	CLEC( CL_IMAGE_FORMAT_MISMATCH         )
	CLEC( CL_IMAGE_FORMAT_NOT_SUPPORTED    )
	CLEC( CL_BUILD_PROGRAM_FAILURE         )
	CLEC( CL_MAP_FAILURE                   )
	CLEC( CL_MISALIGNED_SUB_BUFFER_OFFSET  )
	CLEC( CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST )
	CLEC( CL_INVALID_VALUE )
	CLEC( CL_INVALID_DEVICE_TYPE )
	CLEC( CL_INVALID_PLATFORM )
	CLEC( CL_INVALID_DEVICE )
	CLEC( CL_INVALID_CONTEXT )
	CLEC( CL_INVALID_QUEUE_PROPERTIES )
	CLEC( CL_INVALID_COMMAND_QUEUE )
	CLEC( CL_INVALID_HOST_PTR )
	CLEC( CL_INVALID_MEM_OBJECT )
	CLEC( CL_INVALID_IMAGE_FORMAT_DESCRIPTOR )
	CLEC( CL_INVALID_IMAGE_SIZE )
	CLEC( CL_INVALID_SAMPLER )
	CLEC( CL_INVALID_BINARY )
	CLEC( CL_INVALID_BUILD_OPTIONS )
	CLEC( CL_INVALID_PROGRAM )
	CLEC( CL_INVALID_PROGRAM_EXECUTABLE )
	CLEC( CL_INVALID_KERNEL_NAME )
	CLEC( CL_INVALID_KERNEL_DEFINITION)
	CLEC( CL_INVALID_KERNEL )
	CLEC( CL_INVALID_ARG_INDEX )
	CLEC( CL_INVALID_ARG_VALUE )
	CLEC( CL_INVALID_ARG_SIZE )
	CLEC( CL_INVALID_KERNEL_ARGS )
	CLEC( CL_INVALID_WORK_DIMENSION )
	CLEC( CL_INVALID_WORK_GROUP_SIZE )
	CLEC( CL_INVALID_WORK_ITEM_SIZE )
	CLEC( CL_INVALID_GLOBAL_OFFSET )
	CLEC( CL_INVALID_EVENT_WAIT_LIST )
	CLEC( CL_INVALID_EVENT )
	CLEC( CL_INVALID_OPERATION )
	CLEC( CL_INVALID_GL_OBJECT )
	CLEC( CL_INVALID_BUFFER_SIZE )
	CLEC( CL_INVALID_MIP_LEVEL )
	CLEC( CL_INVALID_GLOBAL_WORK_SIZE )
	CLEC( CL_INVALID_PROPERTY )
	}
	LOG(ERROR) << "OpenCL error @ " << file << "(" << line << ") : " << errname << " " << msg;
}

#undef CLEC