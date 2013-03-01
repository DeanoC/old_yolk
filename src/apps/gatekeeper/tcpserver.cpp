/**
 @file	gatekeeper/tcpserver.cpp

 Implements the tcpserver class.
 */
#include "core/core.h"
#include "protocols/handshake.proto.pb.h"
#include "connection.h"
#include "fsmevents.h"
#include "tcpserver.h"

TcpServer::TcpServer( boost::asio::io_service& io_service,
		    const std::string& address, const int& port ) {
	boost::asio::ip::tcp::resolver resolver(io_service);
	std::stringstream ss;
	ss << port;
	boost::asio::ip::tcp::resolver::query query(address, ss.str() );
	acceptor.reset( CORE_NEW boost::asio::ip::tcp::acceptor(io_service, *resolver.resolve(query)));
}

/**
 Functor called to handle client.
 Calls to this become a co-operative fibered based multi-tasker.
 Parent are spawned by the io_service when an event happens, accept the socket
 and connection and fork a child before existing the function.
 Children handle the connections, (for single true thread when the parents exits)
 at yield points it can switch to other co-op thread, when the child is done 
 it close the socket its been handling and exits the function.
 Multiple threads can attach to the io_service (by run()) to allow pre-emptive 
 threading to occur on top. This ensures HW threads are used in consumption of 
 network input.
 NOTE: parent threads should not use the socket they create in anyway after the
 child fork without mutexs. Currently there is no guarentee of hw thread pinning
 so each yield/reenter may transfer onto a different HW thread, this should only
 be a problem in terms of caches (performance on snooped caches + explicit flush
 on DIY cache snooping platforms).
 */
#include "core/yield.h"
// when called from io_service param == length when called directly can be whatever
// you like
void TcpServer::operator()( boost::system::error_code ec, std::size_t param ) {
   if(!ec) {
		reenter(this) {
			do {
				// TODO socket arena/pool
				connection = std::make_shared<Connection>( acceptor->get_io_service() );

				yield acceptor->async_accept( *connection->getSocket(), *this );

				// kick off a child which will pass the which and execute
				// the handling code below
				fork TcpServer(*this)();

			} while( is_parent() );

			// child thread handles stuff here
			yield connection->process_event( this, FSMEvents::Contact() );
         	yield connection->process_event( this, FSMEvents::GetResponse() );
         	yield connection->process_event( this, FSMEvents::ServiceRecv() );
			if( param == Messages::FirstResponse::CLIENT ) {
			} else if( param == Messages::FirstResponse::DWM ) {
	         	yield connection->process_event( this, FSMEvents::WantDWMService() );
				if( param != 0 ) {
	         		yield connection->process_event( this, FSMEvents::GetResponse() );
	         		yield connection->process_event( this, FSMEvents::HWCapacityRecv() );
				}
			}

			// finished talking, close the connection
			connection.reset();
		}
	}
}
#include "core/unyield.h"
