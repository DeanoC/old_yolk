#pragma once
//!-----------------------------------------------------
//!
//! \file platform.h
//! Contains the main singleton for the cl subsystem
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_PLATFORM_H)
#define WIERD_CL_PLATFORM_H

#include "context.h"

namespace Cl {
	class ProgramMan;
	class Platform : public Core::Singleton<Platform> {
	public:
		void createDevices();
		void destroyDevices();

		ContextPtr	getPrimaryContext() { return primaryContext; }
		size_t		getNumSecondaryContects() const { return secondaryContexts.size(); }
		ContextPtr	getSecondaryContext( int index) { return secondaryContexts[index]; }

		ProgramMan* getProgramMan() const { return programMan.get(); }

	protected:
		friend class Core::Singleton<Platform>;

		Platform();
		~Platform();
		void installResourceTypes();

		static const int MAX_DEVICES = 16;
		static const int MAX_PLATFORMS = 8;

		struct DeviceStruct {
			cl_device_id					id;
			int								rating;
			bool							gpu;
			bool							glShare;
			cl_platform_id					platformId;
			cl_context						context;
			uint32_t						flags;
		};
		std::array<DeviceStruct, MAX_PLATFORMS * MAX_DEVICES>	devices;
		int														totalDeviceCount;
		ContextPtr												primaryContext;
		std::vector<ContextPtr>									secondaryContexts;
		boost::scoped_ptr<ProgramMan>							programMan;
	};
}

#endif // end WIERD_CL_PLATFORM_H