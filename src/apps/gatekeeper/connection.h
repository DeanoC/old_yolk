/**
 @file	connection.h

 Declares the connection class.
 */

#pragma once
#ifndef GATEKEEPER_CONNECTION_H_
#define GATEKEEPER_CONNECTION_H_

#include "gatekeeperfsm.h"
#include "tcpserver.h"

class Connection {
public:
	Connection( boost::asio::io_service& io_service );
	~Connection();

	std::shared_ptr<boost::asio::ip::tcp::socket> getSocket() const { 
		return socket; 
	}
	
	template<class Event> void process_event(TcpServer* server, Event const& evt);

	TcpServer* tmpServer; // internal detail
//	std::function<void(boost::system::error_code, std::size_t)> callback;
private:
	GatekeeperFSMHelper*							gkFSM;

	std::shared_ptr<boost::asio::ip::tcp::socket>	socket;
};

template<class Event>
void Connection::process_event(TcpServer* server, Event const& evt) {
	tmpServer = server;
//	callback = boost::bind(&TcpServer::operator(), boost::ref(server), _1, _2 );
	gkFSM->process_event( evt );
}
#endif