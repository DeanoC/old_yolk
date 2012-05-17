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
	void backmsg( const boost::system::error_code& error );

	boost::asio::io_service&							io_service;
	std::shared_ptr<boost::asio::ip::tcp::socket>		beatSock;
	std::shared_ptr<boost::asio::deadline_timer>		beatTimer;
	int													rate;

	std::array<uint8_t, 1>								beatBuffer;
	std::array<uint8_t, 1>								beatPassageBuffer;
	Core::shared_mutex									dwmMutex;
	std::weak_ptr<Core::thread>							dwm;
};

#endif