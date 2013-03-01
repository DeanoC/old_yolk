/**
 @file	gatekeeper/connection.cpp

 Implements the connection class.
 */
#include "core/core.h"
#include "gatekeeperfsm.h"
#include "tcptunnel.h"
#include "connection.h"

Connection::Connection( boost::asio::io_service& io_service ) :
	tmpServer( nullptr ) {
	socket = std::make_shared<boost::asio::ip::tcp::socket>( io_service );
	gkFSM = CORE_NEW GatekeeperFSMHelper( this );
}

Connection::~Connection() {
	boost::system::error_code ec;
	socket->shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
	socket.reset();
	CORE_DELETE( gkFSM ); gkFSM = nullptr;
}