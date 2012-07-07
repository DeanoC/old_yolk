//!-----------------------------------------------------
//!
//! \file context.h
//! Contains the cl base class for compute memory
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_CONTEXT_H)
#define WIERD_CL_CONTEXT_H

#include "cmdqueue.h"

namespace Cl {

	class Context {
	public:
		cl_context							getContext() { return context; }

		int									getNumDevices() const { return devices.size(); }
		const cl_device_id*					getDevices() const { return &devices[0]; }
		CmdQueuePtr							getCmdQueue( int deviceIndex ) { return cmdQueues[deviceIndex]; }
		const DEVICE_EXTENSIONS_FLAGS*		getDeviceFlags() const { return &deviceFlags[0]; }
		bool								isDeviceGpu( int deviceIndex ) const { return deviceGpus[deviceIndex]; }

		// create a private queue for a device /TODO
		CmdQueuePtr		createCommandQueue( int deviceIndex );

	protected:

		friend class Platform;
		Context( cl_context _con, bool _computeOnly ) : context(_con), computeOnly( _computeOnly ) {}
		void addDevice( cl_device_id id, bool isGpu, DEVICE_EXTENSIONS_FLAGS flags ) {
			devices.push_back( id );
			deviceGpus.push_back( isGpu );
			deviceFlags.push_back( flags );
			cmdQueues.push_back( CmdQueuePtr(CORE_NEW CmdQueue( context, id )) );
		}

		std::vector<CmdQueuePtr>					cmdQueues;
		std::vector<cl_device_id>					devices;
		std::vector<DEVICE_EXTENSIONS_FLAGS>		deviceFlags;
		std::vector<bool>							deviceGpus;

		cl_context		context;
		bool			computeOnly;

	};

	typedef std::shared_ptr<Context>	ContextPtr;

}

#endif //WIERD_CL_CONTEXT_H