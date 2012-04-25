#include "core/core.h"
#include "core/tcp.h"

#include "protocols/handshake.proto.pb.h"

// don't usually do this but makes the statement defs much shorter!
using namespace Core::msm::front;

namespace {
using namespace Core;
using namespace Core::msm; // for front::state (can't use without front as ambigious)
using namespace Core::msm::front; // for Row

// front-end: define the FSM structure 
struct HandshakeStateMachine : public state_machine_def<HandshakeStateMachine> {
	HandshakeStateMachine( const Core::string& addr, const Core::string& port ) :
		m_serverAddr( addr ), m_serverPort( port ) {}

	// list of events
	struct Start {};
	struct Resolved {};
	struct ServiceReply {};
	struct ErrorEvent {};

	// The list of FSM states
	struct Empty : public front::state<> {};

	struct ResolvingHost : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			LOG(INFO) << "Resolving " << fsm.m_serverAddr << ":" << fsm.m_serverPort << "\n";
			// resolve the address and port into possible socket endpoints
			asio::ip::tcp::resolver netResolver( fsm.m_ioService );
			asio::ip::tcp::resolver::query endPoint( fsm.m_serverAddr, fsm.m_serverPort );
			asio::ip::tcp::resolver::iterator epIter = netResolver.resolve( endPoint );

			// find the first valid endpoint that wants to communicate with us
			TcpConnection::pointer connection = TcpConnection::create( fsm.m_ioService );
			const asio::ip::tcp::resolver::iterator endPointEnd;
			system::error_code err = asio::error::host_not_found;
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
				fsm.m_connection = connection;
				fsm.process_event( Resolved() );
			}
		}
	};

	struct Hello : public front::state<> {
		template <class EVT,class FSM>
		void on_entry(EVT const& ,FSM& fsm ) {
			size_t readSize = fsm.m_connection->syncRead( fsm.buffer.data(), fsm.buffer.size() );
			Messages::FirstContact fc;
			fc.ParseFromArray( fsm.buffer.data(), readSize );
			CORE_ASSERT( fc.has_clientstring() );
			CORE_ASSERT( fc.has_magicid() );
			CORE_ASSERT( fc.magicid() == 0xDEADDEAD );
			LOG(INFO) << "Contact made with Server : " << fc.clientstring();
			fsm.process_event( ServiceReply() );
		}
	};

	struct DWMDediRequest : public front::state<> {
		template <class EVT,class FSM> void on_entry(EVT const& ,FSM& fsm) {
			Messages::FirstResponse fr;
			fr.Clear();
			fr.set_service( Messages::FirstResponse_SERVICE_DWM );
			fr.SerializeToArray( fsm.buffer.data(), fsm.buffer.size() );
			fsm.m_connection->syncWrite( fsm.buffer.data(), fr.ByteSize() );
		}
	};

	struct AllOk : public front::state<> {};
	// this state is also made terminal so that all the events are blocked
	struct ErrorMode : public terminate_state<> {};

	template <class Fsm,class Event> void no_transition(Event const& e, Fsm& ,int state) {
		LOG(INFO) << "no transition from state " << state << " on event " << typeid(e).name() << "\n";
	}

	template <class Fsm,class Event> void exception_caught(Event const& e,Fsm& fsm, std::exception&) {
		fsm.process_event( ErrorEvent() );
	}

	// the initial state of the player SM. Must be defined
	// two regions the standard state (0) and the error state (1)
	typedef Core::mpl::vector< Empty, AllOk > initial_state;

	// Transition table for player
	struct transition_table : Core::mpl::vector <
// +--------------+-----------------+---------------+---------------+-----------+
// |    Start     |  Event			|     Next		|     Action    |  Guard    |
// +--------------+-----------------+---------------+---------------+-----------+
Row< Empty        , Start			, ResolvingHost	, none			, none		>, 
Row< ResolvingHost, Resolved		, Hello			, none			, none		>,
Row< Hello		  , ServiceReply	, DWMDediRequest, none			, none		>, 
// +--------------+-----------------+---------------+---------------+-----------+
Row< AllOk		  , ErrorEvent		, ErrorMode		, none			, none		> 
// +--------------+-----------------+---------------+---------------+-----------+
> {};

	Core::asio::io_service			m_ioService;	//!< The i/o service object
	Core::string					m_serverAddr;	//!< The server address string
	Core::string					m_serverPort;	//!< The server port string
	TcpConnection::pointer			m_connection;	//!< Connection to the server once resolved
	std::array<uint8_t, 1024*8>		buffer;

};

} // end namespace 

///-------------------------------------------------------------------------------------------------
/// \fn	bool GameServer::Utils::HandshakeWithMcp()
/// \return	true if it succeeds, false if it fails. 
////////////////////////////////////////////////////////////////////////////////////////////////////
bool Handshake( const Core::string& addr, const int& port ) {
   std::stringstream ss;
   ss << port;

	Core::msm::back::state_machine<HandshakeStateMachine> sm( addr, ss.str() );

	// needed to start the highest-level SM. This will call on_entry and mark the start of the SM
	sm.start();
	sm.process_event( HandshakeStateMachine::Start() );

	return ( sm.is_flag_active<Core::msm::TerminateFlag>() );
}
