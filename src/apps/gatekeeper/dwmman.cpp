/**
 @file	dwmman.cpp

 Implments the dwmman class.
 */
#include "core/core.h"
#include "dwmman.h"
#include "heartbeat.h"

DWMMan::DWMMan() {
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
	inactiveDWMs.reserve( 1000 );
}

void DWMMan::addNewDWM( const boost::asio::ip::address& addr ) { 
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
	inactiveDWMs.push_back( addr );
}

bool DWMMan::isAreaActive( int area ) { 
	Core::shared_lock< Core::shared_mutex > writerLock( dwmMutex );
	return activeDWMs.find( area) != activeDWMs.end(); 
}
const boost::asio::ip::address DWMMan::activateDWMForArea( int area ) {
	Core::unique_lock< Core::shared_mutex > writerLock( dwmMutex );
	assert( activeDWMs.find( area ) == activeDWMs.end() );

	boost::asio::ip::address noAddr = boost::asio::ip::address();
	boost::asio::ip::address dwmAddy;
	do {
		dwmAddy = inactiveDWMs.empty() ? noAddr : inactiveDWMs.back();
		if( dwmAddy != noAddr ) {
			inactiveDWMs.pop_back();
		}
	} while( dwmAddy != noAddr && HeartBeat::checkAlive( dwmAddy ) == false );

	if( dwmAddy != noAddr ) {
		activateArea( dwmAddy, area );
	} 
	return dwmAddy;
}
void DWMMan::activateArea( const boost::asio::ip::address& addr, const int area ) {
	activeDWMs[ area ] = std::make_shared<DWMChan>( addr, area );

	LOG(INFO) << "Area " << area << " activated\n";
}