/**
 @file	dwmman.h

 Declares the dwmman class.
 */

#pragma once
#ifndef GATEKEEPER_DWMMAN_H_
#define GATEKEEPER_DWMMAN_H_

#include "core/singleton.h"

class DWMMan : public Core::Singleton<DWMMan> {
public:
	DWMMan() {
		Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
		inactiveDWMs.reserve( 1000 );
	}

	void addNewDWM( const boost::asio::ip::address& addr ) { 
		Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
		inactiveDWMs.push_back( addr );
	}

	bool isAreaActive( int area ) { 
		Core::shared_lock< Core::shared_mutex > writerLock( dwmMutex );
		return activeDWMs.find( area) != activeDWMs.end(); 
	}
	size_t numInactiveDWMs() const { return inactiveDWMs.size(); }

	const boost::asio::ip::address activateDWMForArea( int area ) {
		Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
		assert( activeDWMs.find( area) == activeDWMs.end() );

		auto ret = inactiveDWMs.back();
		inactiveDWMs.pop_back();
		activeDWMs[ area ] = ret;
		LOG(INFO) << "Area " << area << " activated\n";
		return ret;
	}

private:
	Core::shared_mutex								dwmMutex;
	std::map< int, boost::asio::ip::address >		activeDWMs;
	std::vector< const boost::asio::ip::address >	inactiveDWMs;
};

#endif