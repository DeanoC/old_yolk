#include "core/core.h"
#include "core/tcp.h"

#include "protocols/handshake.proto.pb.h"


namespace {
namespace asio = boost::asio;
using namespace boost::msm; // for front::state (can't use without front as ambigious)
using namespace boost::msm::front; // for Row

// front-end: define the FSM structure 
struct HandshakeStateMachine : public state_machine_def<HandshakeStateMachine>, private boost::noncopyable {
	HandshakeStateMachine( boost::asio::io_service* _io, const std::string& addr, const std::string& port ) :
		 io_service( *_io ), serverAddr( addr ), serverPort( port ) {}

	// list of events
	struct Start {};
	struct Resolved {};
	struct ServiceReply {};

	struct ErrorEvent {};

	// The list of FSM states
	struct Empty : public front::state<> {};

	struct ResolvingHost : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "Resolving " << fsm.serverAddr << ":" << fsm.serverPort << "\n";
			// resolve the address and port into possible socket endpoints
			asio::ip::tcp::resolver netResolver( fsm.io_service );
			asio::ip::tcp::resolver::query endPoint( fsm.serverAddr, fsm.serverPort );
			asio::ip::tcp::resolver::iterator epIter = netResolver.resolve( endPoint );

			// find the first valid endpoint that wants to communicate with us
			Core::TcpConnection::pointer connection = Core::TcpConnection::create( fsm.io_service );
			const asio::ip::tcp::resolver::iterator endPointEnd;
			boost::system::error_code err = asio::error::host_not_found;
			while (err && epIter != endPointEnd )
			{
				connection->socket().close();
				connection->socket().connect(*epIter, err);
				++epIter;
			}
			// nobody wanted to talk to us, so handshake fail
			if (err) {
				LOG(INFO) << "Resolve error : " << err.message();
				fsm.process_event( ErrorEvent() );
			} else {
				fsm.connection = connection;
				fsm.process_event( Resolved() );
			}
		}
	};

	struct Hello : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm ) {
			size_t readSize = fsm.connection->syncRead( fsm.buffer.data(), fsm.buffer.size() );
			Messages::FirstContact fc;
			fc.ParseFromArray( fsm.buffer.data(), readSize );
			CORE_ASSERT( fc.has_clientstring() );
			CORE_ASSERT( fc.has_magicid() );
			CORE_ASSERT( fc.magicid() == 0xDEADDEAD );
			LOG(INFO) << "Contact made with Server : " << fc.clientstring();
			fsm.process_event( ServiceReply() );
		}
	};

	struct ClientRequest : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Messages::FirstResponse fr;
			fr.set_service( Messages::FirstResponse::CLIENT );
			fr.set_id( 0x0 ); // special dummy client id
			fr.SerializeToArray( fsm.buffer.data(), fsm.buffer.size() );
			fsm.connection->syncWrite( fsm.buffer.data(), fr.ByteSize() );
		}
	};

	// the two main states, ok or error, any exception switches to error which is a terminition condition of the fsm
	struct AllOk : public front::state<> {};
	struct ErrorMode : public terminate_state<> {}; 	///< this state is also made terminal so that all the events are blocked


	/// development error, state can't handle the event thats been fired
	template <class Fsm,class Event> void no_transition(Event const& e, Fsm& ,int state) {
		LOG(INFO) << "no transition from state " << state << " on event " << typeid(e).name() << "\n";
	}

	/// exchange an exception into an Error event
	template <class Fsm,class Event> void exception_caught(Event const& e,Fsm& fsm, std::exception&) {
		fsm.process_event( ErrorEvent() );
	}

	// the initial state of the FSM. Must be defined for each main states
	typedef boost::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for DWM
	struct transition_table : boost::mpl::vector <
// +--------------------+-------------------+-------------------+---------------+-----------+
// |    State			|  Event			|     Next			|     Action    |  Guard    |
// +--------------------+-------------------+-------------------+---------------+-----------+
Row< Empty				, Start				, ResolvingHost		, none			, none		>, 
Row< ResolvingHost		, Resolved			, Hello				, none			, none		>,
Row< Hello				, ServiceReply		, ClientRequest		, none			, none		>, 
// +--------------+-----------------+---------------+---------------+-----------+
Row< AllOk		  , ErrorEvent		, ErrorMode		, none			, none		> 
// +--------------+-----------------+---------------+---------------+-----------+
> {};

	boost::asio::io_service&		io_service;	//!< The i/o service object
	std::string						serverAddr;	//!< The server address string
	std::string						serverPort;	//!< The server port string
	Core::TcpConnection::pointer			connection;	//!< Connection to the server once resolved
	std::array<uint8_t, 1024*8>		buffer;
};

} // end namespace 

///-------------------------------------------------------------------------------------------------
/// \fn	bool GameServer::Utils::HandshakeWithMcp()
/// \return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Handshake( boost::asio::io_service& io_service, const std::string& addr, int port ) {

	std::stringstream ss;
	ss << port;
	boost::msm::back::state_machine<HandshakeStateMachine> sm( &io_service, addr, ss.str() );

	// needed to start the highest-level SM. This will call on_entry and mark the start of the SM
	sm.start();
	sm.process_event( HandshakeStateMachine::Start() );

	return ( !sm.is_flag_active<boost::msm::TerminateFlag>() );
}
