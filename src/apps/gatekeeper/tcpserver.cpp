/**
 @file	gatekeeper/tcpserver.cpp

 Implements the tcpserver class.
 */
#include "core\core.h"
#include "tcpserver.h"
#include "protocols/handshake.proto.pb.h"
// don't usually do this but makes the statement defs much shorter!
using namespace Core::msm::front;

TcpServer::TcpServer( boost::asio::io_service& io_service,
		    const std::string& address, const int& port ) {
   hider = nullptr;
	tcp::resolver resolver(io_service);
   std::stringstream ss;
   ss << port;
	tcp::resolver::query query(address, ss.str() );
	acceptor.reset(new tcp::acceptor(io_service, *resolver.resolve(query)));
}
#include "core/yield.h"


///-------------------------------------------------------------------------------------------------
/// \struct	ServerHandshakeFSM
///
/// \brief	handshake finite state machine. 
///
////////////////////////////////////////////////////////////////////////////////////////////////////
struct ServerHandshakeFSM : public state_machine_def<ServerHandshakeFSM> {

	ServerHandshakeFSM( std::shared_ptr<Core::asio::ip::tcp::socket>& connection ) :
		m_connection( connection ) {}

	// list of events
	struct Contact {};
	struct Response {};
	struct ErrorEvent {};

	// The list of FSM states  
	struct Empty : public state<> {};

	struct FirstContact : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Messages::FirstContact fc;
			fc.Clear();
			fc.set_magicid( 0xDEADDEAD );
			fc.set_clientstring( "MCP" );
			Core::array< uint8_t, 1024 > buffer;
			fc.SerializeToArray( buffer.data(), buffer.size() );
         
			LOG(INFO) << "First Contact sent";

         const int size = fc.ByteSize();
         Core::asio::write( *fsm.m_connection, Core::asio::buffer( &size, 4 ) );
		   Core::asio::write( *fsm.m_connection, Core::asio::buffer( buffer.data(), size ) );

			fsm.process_event( Response() );
		}
	};

	struct FirstResponse : public state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Core::string message;
//			m_connection->SyncRead( message );
		}
	};

	struct AllOk : public state<> {};
	// this state is also made terminal so that all the events are blocked
	struct ErrorMode : public terminate_state<> {};

	template <class Fsm,class Event> void no_transition(Event const& e, Fsm& ,int state) {
		LOG(INFO) << "no transition from state " << state << " on event " << typeid(e).name() << "\n";
	}

	template <class Fsm,class Event> void exception_caught(Event const& e,Fsm& fsm, std::exception&) {
		fsm.process_event( ErrorEvent() );
	}

	// the initial state of the SM. Must be defined
	// two regions the standard state (0) and the error state (1)
	typedef Core::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for player
	struct transition_table : Core::mpl::vector <
// +--------------+-----------------+-----------------+-----------+--------+
// |    Start     |      Event      |      Next       |  Action   | Guard  |
// +--------------+-----------------+-----------------+-----------+--------+
Row< Empty        , Contact         , FirstContact    , none      , none   >, 
Row< FirstContact , Response        , FirstResponse   , none      , none   >,
Row< AllOk		   , ErrorEvent	   , ErrorMode       , none      , none   > 
// +--------------+-----------------+-----------------+-----------+--------+
> {};

	std::shared_ptr<Core::asio::ip::tcp::socket>	m_connection;	//!< The connection
};

class FSMHider {
public:
   FSMHider( std::shared_ptr<Core::msm::back::state_machine<ServerHandshakeFSM> > _fsm ) :
      fsm(_fsm) {}
   
   std::shared_ptr<Core::msm::back::state_machine<ServerHandshakeFSM> > fsm;
};

TcpServer::~TcpServer() {
   if( hider != nullptr ) {
      CORE_DELETE hider;
   }
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
void TcpServer::operator()( boost::system::error_code ec, std::size_t length ) {
   
   std::shared_ptr<Core::msm::back::state_machine<ServerHandshakeFSM> > tfsm;

   if(!ec) {
		reenter(this) {
			do {
				// TODO socket arena/pool
				socket.reset( CORE_NEW tcp::socket( acceptor->get_io_service() ) );

				yield acceptor->async_accept( *socket, *this );

				// kick off a child which will pass the which and execute
				// the handling code below
				fork TcpServer(*this)();

			} while( is_parent() );

         tfsm.reset( CORE_NEW Core::msm::back::state_machine<ServerHandshakeFSM>(socket) );
         hider = CORE_NEW FSMHider( tfsm );

			// child thread handles stuff here
	      hider->fsm->process_event( ServerHandshakeFSM::Contact() );
         
			// finished talking, close the socket
			socket->shutdown( tcp::socket::shutdown_both, ec );
		}
	}
}

#include "core/unyield.h"
