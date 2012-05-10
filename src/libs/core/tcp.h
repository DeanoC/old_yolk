///-------------------------------------------------------------------------------------------------
/// \file	tcp.h
///
/// \brief	Declares the tcp class. 
///
/// \details	
///		tcp description goes here
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORE_TCP_H_
#define CORE_TCP_H_

namespace Core {

class TcpConnection
	: public Core::enable_shared_from_this<TcpConnection> {
public:
	typedef Core::shared_ptr<TcpConnection> pointer;

	static pointer create( boost::asio::io_service& ioService ) {
		return pointer( CORE_NEW TcpConnection( ioService ) );
	}

	boost::asio::ip::tcp::socket& socket() {
		return sock;
	}

	void syncWrite( const uint8_t* buffer, const size_t size ) {
		boost::asio::write( sock, Core::asio::buffer( &size, 4 ) );
		boost::asio::write( sock, Core::asio::buffer( buffer, size ) );
	}
	void syncWrite( const Core::string& buffer ) {
		const size_t size = buffer.size();
		boost::asio::write( sock, Core::asio::buffer( &size, 4 ) );
		boost::asio::write( sock, Core::asio::buffer( buffer ) );
	}

	size_t syncRead( uint8_t* buffer, const size_t maxSize ) {
		uint32_t serverSize;
		boost::asio::read( sock, Core::asio::buffer( &serverSize, 4 ) );
		CORE_ASSERT( serverSize < maxSize );
		uint32_t clientSize;
		clientSize = boost::asio::read( sock, Core::asio::buffer( buffer, serverSize ) );
		CORE_ASSERT( clientSize == serverSize );
		return clientSize;
	}

private:
	TcpConnection( boost::asio::io_service& ioService )
		: sock( ioService ) {}	

	boost::asio::ip::tcp::socket sock;
};

class TcpSimpleServer {
public:
	typedef void (*ConnectionFunc)( TcpConnection::pointer );

	TcpSimpleServer(boost::asio::io_service& ioService, uint16_t port, ConnectionFunc func ) : 
		m_acceptor(ioService, boost::asio::ip::tcp::endpoint( Core::asio::ip::tcp::v4(), port) ), 
		m_func( func ) {
	}

	void start() {
		// TODO exit message
		while( true ) {
			// create new connection
			TcpConnection::pointer newConn = TcpConnection::create( m_acceptor.get_io_service() );

			// block until we get an incoming message
			m_acceptor.accept( newConn->socket() );
			// TODO use TBB task pools rather than thread per connection!
//			Core::thread thrd( m_func, Core::ref( newConn ) );
//			thrd.detach();
			m_func( newConn );
		}
	}
	
private:
	Core::asio::ip::tcp::acceptor	m_acceptor;
	ConnectionFunc					m_func;
};

}; // end namspace Core;

#endif // CORE_TCP_H_
