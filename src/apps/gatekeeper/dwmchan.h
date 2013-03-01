/**
 @file	dwmchan.h

 Declares the dwmchan class.
 */

#pragma once
#ifndef GATEKEEPER_DWMCHAN_H_
#define GATEKEEPER_DWMCHAN_H_

#include "tcptunnel.h"

#define DWM_CHAN_PORT 5002
 
class DWMChan {
public:
	DWMChan( const boost::asio::ip::address& addr, const int area );

	void useBackPassage();
	void accept( std::shared_ptr<boost::asio::ip::tcp::socket> incoming );
private:
	typedef std::shared_ptr<boost::asio::ip::tcp::socket> 		SocketPtr;

	const boost::asio::ip::address addr;
	const int area;
	SocketPtr socket;

//	std::scoped_ptr< TcpTunnel >		tunnel;

};

#endif