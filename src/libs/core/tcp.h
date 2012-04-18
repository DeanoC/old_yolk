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

class TcpConnection
	: public Core::enable_shared_from_this<TcpConnection> {
public:
	typedef Core::shared_ptr<TcpConnection> pointer;

	static pointer create( Core::asio::io_service& ioService ) {
		return pointer( CORE_NEW TcpConnection( ioService ) );
	}

	Core::asio::ip::tcp::socket& socket() const {
		return m_socket;
	}

	void syncWrite( const uint8_t* buffer, const size_t size ) {
		Core::asio::write( m_socket, Core::asio::buffer( &size, 4 ) );
		Core::asio::write( m_socket, Core::asio::buffer( buffer, size ) );
	}
	void syncWrite( const Core::string& buffer ) {
		const size_t size = buffer.size();
		Core::asio::write( m_socket, Core::asio::buffer( &size, 4 ) );
		Core::asio::write( m_socket, Core::asio::buffer( buffer ) );
	}

	size_t syncRead( uint8_t* buffer, const size_t maxSize ) {
		size_t serverSize;
		Core::asio::read( m_socket, Core::asio::buffer( &serverSize, 4 ) );
		CORE_ASSERT( serverSize < maxSize );
		size_t clientSize;
		clientSize = Core::asio::read( m_socket, Core::asio::buffer( buffer, serverSize ) );
		CORE_ASSERT( clientSize == serverSize );
		return clientSize;
	}

private:
	TcpConnection( Core::asio::io_service& ioService )
		: m_socket( ioService ) {}	

	Core::asio::ip::tcp::socket m_socket;
};

class TcpSimpleServer {
public:
	typedef void (*ConnectionFunc)( TcpConnection::pointer );

	TcpSimpleServer(Core::asio::io_service& ioService, uint16_t port, ConnectionFunc func ) : 
		m_acceptor(ioService, Core::asio::ip::tcp::endpoint( Core::asio::ip::tcp::v4(), port) ), 
		m_func( func ) {
	}

	void start() {
		// TODO exit message
		while( true ) {
			// create new connection
			TcpConnection::pointer newConn = TcpConnection::create( m_acceptor.io_service() );

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

#endif // CORE_TCP_H_
