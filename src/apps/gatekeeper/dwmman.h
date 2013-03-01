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

	bool isAreaActive( int area );
	size_t numInactiveDWMs() const { return inactiveDWMs.size(); }

	void addNewDWM( boost::asio::ip::address addr );

	const boost::asio::ip::address activateDWMForArea( int area );
	void accept( boost::asio::io_service& io_service, const std::string& addr );
private:
	typedef std::shared_ptr<boost::asio::ip::tcp::socket>				SocketPtr;
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor>				AcceptorPtr;
	void activateArea( const boost::asio::ip::address& dwmAddy, const int area );

	void accept();

	std::shared_ptr<boost::asio::io_service>							ioService;
	AcceptorPtr															acceptor;

	Core::shared_mutex													dwmMutex;
	std::map< int, std::shared_ptr<DWMChan> >							activeDWMsByArea;
	std::map< boost::asio::ip::address, std::shared_ptr<DWMChan> >		activeDWMsByAddr;
	std::vector< boost::asio::ip::address >								inactiveDWMs;
};

#endif