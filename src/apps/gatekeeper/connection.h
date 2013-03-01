/**
 @file	connection.h

 Declares the connection class.
 */

#pragma once
#ifndef GATEKEEPER_CONNECTION_H_
#define GATEKEEPER_CONNECTION_H_

#include "gatekeeperfsm.h"
class TcpServer;
class TcpTunnel;

class Connection {
public:
	Connection( boost::asio::io_service& io_service );
	~Connection();

	std::shared_ptr<boost::asio::ip::tcp::socket> getSocket() const { 
		return socket; 
	}
	
	template<class Event> void process_event(TcpServer* server, Event const& evt);

	TcpServer*	tmpServer; // internal detail
private:
	GatekeeperFSMHelper*							gkFSM;
	std::shared_ptr<boost::asio::ip::tcp::socket>	socket;
};

template<class Event>
void Connection::process_event(TcpServer* server, Event const& evt) {
	tmpServer = server;
	gkFSM->process_event( evt );
}
#endif