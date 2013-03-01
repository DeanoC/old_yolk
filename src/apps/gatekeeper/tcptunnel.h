/**
 @file	tcptunnel.h

 Declares the tcptunnel class.
 */

#pragma once
#ifndef GATEKEEPER_TCPTUNNEL_H_
#define GATEKEEPER_TCPTUNNEL_H_

#include "core/coroutine.h"
#include "heartbeat.h"

class TcpTunnel : public coroutine {
public:
	TcpTunnel( std::shared_ptr<boost::asio::ip::tcp::socket> r, std::shared_ptr<boost::asio::ip::tcp::socket> w );
	void operator() ( boost::system::error_code ec, std::size_t length );

private:
	void chk( const boost::system::error_code& error );

	std::shared_ptr<boost::asio::ip::tcp::socket>		reader;
	std::shared_ptr<boost::asio::ip::tcp::socket>		writer;

	std::array<uint8_t, 8096>							buffer;
//	std::shared_ptr<HeartBeat>							heart;
	std::shared_ptr<boost::asio::deadline_timer>		chkTimer;
};

class BiTunnel : public coroutine {
public:
	BiTunnel( std::shared_ptr<boost::asio::ip::tcp::socket> a, std::shared_ptr<boost::asio::ip::tcp::socket> b );
private:
	TcpTunnel						a2c;
	TcpTunnel						c2a;
};
#endif