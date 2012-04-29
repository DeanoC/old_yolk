/**
 @file	dwmman.h

 Declares the dwmman class.
 */

#pragma once
#ifndef GATEKEEPER_DWMMAN_H_
#define GATEKEEPER_DWMMAN_H_

#include "core/singleton.h"
#include "dwmchan.h"

class DWMMan : public Core::Singleton<DWMMan> {
public:
	DWMMan();

	void addNewDWM( boost::asio::ip::address addr );

	bool isAreaActive( int area );

	size_t numInactiveDWMs() const { return inactiveDWMs.size(); }

	const boost::asio::ip::address activateDWMForArea( int area );

	std::shared_ptr<boost::asio::io_service> getIoService() const { return ioService; }

	void setIoService( std::shared_ptr<boost::asio::io_service> service ) { ioService = service; }
private:
	void activateArea( const boost::asio::ip::address& dwmAddy, const int area );

	std::shared_ptr<boost::asio::io_service>			ioService;

	Core::shared_mutex								dwmMutex;
	std::map< int, std::shared_ptr<DWMChan> >		activeDWMs;
	std::vector< boost::asio::ip::address >	inactiveDWMs;
};

#endif