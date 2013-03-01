/**
 @file	heartbeat.h

 Declares the heartbeat class.
 */

#pragma once
#ifndef GATEKEEPER_HEARTBEAT_H_
#define GATEKEEPER_HEARTBEAT_H_

#define HB_PORT 5001
#define HB_SECONDS 5
#define HB_ITS_DEAD_JIM 8

#include "core/singleton.h"

class HeartBeat : public Core::Singleton<HeartBeat> {
public:
	typedef std::shared_ptr<boost::asio::ip::tcp::socket> 		SocketPtr;
	void accept( boost::asio::io_service& io_service, const std::string& addr );

	bool checkAlive( const boost::asio::ip::address& addr );
	SocketPtr getBeatingHeart( const boost::asio::ip::address& addr );

private:
	typedef std::shared_ptr<boost::asio::deadline_timer> 		DeadLineTimerPtr;
	typedef std::shared_ptr<boost::asio::ip::tcp::acceptor>		AcceptorPtr;
	typedef std::map<boost::asio::ip::address, std::pair<int, SocketPtr>> BeatMap;

	void accept();
	void beat( SocketPtr socket );
	void tick( const boost::system::error_code& error );

	DeadLineTimerPtr									timer;
	std::array<uint8_t, 4096>							buffer;

	boost::asio::io_service* 							ioService;
	std::shared_ptr<boost::asio::ip::tcp::acceptor>		acceptor;
	int													beatCount;
	Core::shared_mutex									mapMutex;
	BeatMap												ip2Alive;
};

#endif