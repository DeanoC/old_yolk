/**
 @file	heart.h

 Declares the heartbeat class.
 */

#pragma once
#ifndef DWM_DEDI_HEART_H_
#define DWM_DEDI_HEART_H_

class Heart {
public:
	Heart( boost::asio::io_service& io_service );

	void start( const std::string& addr, int port, int rate );

private:
	void tick ( const boost::system::error_code& error );

	boost::asio::io_service&							io_service;
	std::shared_ptr<boost::asio::ip::udp::socket>		beatSock;
	std::shared_ptr<boost::asio::deadline_timer>		beatTimer;
	boost::asio::ip::udp::endpoint						beatRemoteEndpoint;
	std::array<uint8_t, 128>							beatBuffer;
	int													rate;
	boost::asio::ip::udp::endpoint						target;
};

#endif