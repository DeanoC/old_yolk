/**
 @file	gatekeeper/tcptunnel.cpp

 Implements the tcptunnel class.
 */
#include "core/core.h"
#include "protocols/handshake.proto.pb.h"
#include "connection.h"
#include "fsmevents.h"
#include "tcptunnel.h"


TcpTunnel::TcpTunnel( std::shared_ptr<boost::asio::ip::tcp::socket> r, std::shared_ptr<boost::asio::ip::tcp::socket> w ) :  
	reader( r ),
	chkTimer( std::make_shared<boost::asio::deadline_timer>( r->get_io_service() ) )
//	, heart( std::make_shared<HeartBeat>( r->get_io_service() ) )
{

	chkTimer->expires_from_now( boost::posix_time::seconds( 120 ) );
	chkTimer->async_wait( boost::bind(&TcpTunnel::chk, this, boost::asio::placeholders::error) );

	// kick it off (its all async so this call will return and then we be re-entered (possible on a different thread)
	// as required by the data transfers from r
	operator()( boost::system::error_code(), 0 );
}
#include "core/yield.h"
void TcpTunnel::operator()( boost::system::error_code ec, std::size_t len ) {
	namespace asio = boost::asio;

	// this is an overlapped half-duplex tunnel, read from R, copy to W
	// full duplex has two of these flipped (R -> W && W -> R)
	if(!ec) {
		reenter(this) {
			do {
				yield asio::async_read( *reader, asio::buffer(buffer), *this );
				fork TcpTunnel( *this )( boost::system::error_code(), 0 );
			} while( is_parent() );

			yield asio::async_write( *writer, asio::buffer(buffer,len), *this );
		}
	}
}
#include "core/unyield.h"

void TcpTunnel::chk( const boost::system::error_code& error ) {
//	if( heart->checkAlive( reader->remote_endpoint().address() ) == true ) 
	{
		chkTimer->async_wait( boost::bind(&TcpTunnel::chk, this, boost::asio::placeholders::error) );
	} 
	//else 
	{
		// TODO kill ourselves! we only do reader, as we assume our pair is doing writer... should be an option
	}
}



BiTunnel::BiTunnel( std::shared_ptr<boost::asio::ip::tcp::socket> a, std::shared_ptr<boost::asio::ip::tcp::socket> c ) :
	a2c( a, c ), c2a( c, a )
{
	// both tunnels are now running async copying data efficiently
	// TODO kill ourself when done...
}

