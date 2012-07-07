/**
 @file	fsmevents.h

 Declares the fsmevents class.
 */

#pragma once
#ifndef GATEKEEPER_FSMEVENTS_H_
#define GATEKEEPER_FSMEVENTS_H_

namespace FSMEvents {
	// list of events
	struct Contact {};
	struct GetResponse {};
	struct ServiceRecv {};
	struct ErrorEvent {};
	struct WantClientService {
		uint64_t id;
	};	
	struct WantDWMService {};	
	struct HWCapacityRecv {};
};

#endif