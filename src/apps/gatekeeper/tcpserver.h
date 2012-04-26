/**
 @file	Z:\Projects\servers\halws\src\bin\lb\tcpserver.h

 Declares the tcpserver class.
 */

#pragma once
#ifndef GATEKEEPER_TCPSERVER_H_
#define GATEKEEPER_TCPSERVER_H_


#include "core/coroutine.h"

class Connection;


class TcpServer : public coroutine {
public:
	/// Construct the server to listen on the specified TCP address and port, and
	/// serve up files from the given directory.
	explicit TcpServer(boost::asio::io_service& io_service,
		const std::string& address, const int& port );

	/// Perform work associated with the server.
	void operator()(
		boost::system::error_code ec = boost::system::error_code(),
		std::size_t length = 0);

private:
	std::shared_ptr<boost::asio::ip::tcp::acceptor>		acceptor; 	///< Acceptor used to listen for incoming connections.
	std::shared_ptr<Connection>							connection; ///< Socket and FSM 

};


#endif