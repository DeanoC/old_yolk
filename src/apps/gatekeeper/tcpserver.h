/**
 @file	Z:\Projects\servers\halws\src\bin\lb\tcpserver.h

 Declares the tcpserver class.
 */

#pragma once
#ifndef GATEKEEPER_TCPSERVER_H_
#define GATEKEEPER_TCPSERVER_H_


#include "core/coroutine.h"

class HandShakeFSM;
class TcpServer;

class ClientConnection {
public:
   ClientConnection( boost::asio::io_service& io_service );
   ~ClientConnection();

	std::shared_ptr<boost::asio::ip::tcp::socket> getSocket() const { return socket; }
	template<class Event>
	void process_event(TcpServer* server, Event const& evt);

	TcpServer* tmpServer; // internal detail
private:
   // note: we use naked pointers, so that we can decompile the FSM met-program fromt headers
	HandShakeFSM*									fsm;
	std::shared_ptr<boost::asio::ip::tcp::socket>	socket;
};

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
	std::shared_ptr<ClientConnection>					connection; ///< Socket and FSM 
	std::shared_ptr<std::array<char, 8192> >			buffer; 	   ///< Buffer for incoming data.

};


#endif