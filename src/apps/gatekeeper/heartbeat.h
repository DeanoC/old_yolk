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

class HeartBeat {
	public:
	HeartBeat( boost::asio::io_service& io_service );

	static bool checkAlive( const boost::asio::ip::address& addr );
	static int getReturnPort( const boost::asio::ip::address& addr );
private:

	void beat(const boost::system::error_code& error);
	void tick ( const boost::system::error_code& error );

	std::shared_ptr<boost::asio::ip::udp::socket>		beatSock;
	std::shared_ptr<boost::asio::deadline_timer>		beatTimer;
	boost::asio::ip::udp::endpoint						beatRemoteEndpoint;
	std::array<uint8_t, 4>								beatBuffer;
	static int											beatCount;
	static Core::shared_mutex							mapMutex;
	static std::map< boost::asio::ip::address, std::pair<int, int> >	ip2Alive;
};

#endif