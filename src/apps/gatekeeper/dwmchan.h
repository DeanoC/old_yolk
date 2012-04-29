/**
 @file	dwmchan.h

 Declares the dwmchan class.
 */

#pragma once
#ifndef GATEKEEPER_DWMCHAN_H_
#define GATEKEEPER_DWMCHAN_H_

#include "tcptunnel.h"

class DWMChan {
public:
	DWMChan( const boost::asio::ip::address& addr, const int area );
private:
	std::shared_ptr< boost::asio::ip::udp::socket > backPassage; //!< UDP heartbeat back channel
	boost::asio::ip::udp::endpoint					bpEndpoint;
	std::array< uint8_t, 1>							bpBuf;

//	std::scoped_ptr< TcpTunnel >		tunnel;

};

#endif