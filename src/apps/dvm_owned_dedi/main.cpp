#include "core/core.h"
#include "dwm/dwm.h"
#include "dwm/bitcoder.h"
#include "dwm/mmu.h"
#include "dwm_client/client.h"

#include "heart.h"
#include "boost/program_options.hpp"
#include "json_spirit/json_spirit_reader.h"
#include "handshake.h"

std::string 								hostname( "127.0.0.1" );
int 										port( 2045 );
std::shared_ptr<boost::asio::io_service>	io;
std::shared_ptr<Heart>						g_heart;

void readConfig( std::string& hostname, int& port ) {
   std::ifstream is( "./config.json" );
   if( is.bad() || !is.is_open() )
      return;
   
   json_spirit::Value value;

   json_spirit::read( is, value );
   if( value.is_null() )
      return;

   auto obj = value.get_obj();
   for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
      if( val->name_ == "hostname" ) {
         hostname = val->value_.get_str();
      } else if( val->name_ == "port" ) {
         port = val->value_.get_int();
      }
   }
}

int Main() {
	using namespace Core;
	namespace po = boost::program_options;

	BitCoder::init();
	MMU::init();

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
	;
	po::variables_map vm;
	po::store(po::parse_command_line(Core::g_argc, Core::g_argv, desc), vm);
	po::notify(vm);    

	if (vm.count("help")) {
		LOG(INFO) << desc << "\n";
		return 1;
	}

	readConfig( hostname, port );

	io = std::make_shared<boost::asio::io_service>();
	g_heart = std::make_shared<Heart>( *io );


	// Wait for signals indicating time to shut down.
	boost::asio::signal_set signals( *io );
	signals.add(SIGINT);
	signals.add(SIGTERM);
#if defined(SIGQUIT)
	signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
	signals.async_wait( boost::bind( 
				&boost::asio::io_service::stop, &(*io) ) );

	// Create a pool of threads to run all of the io_services.
	std::vector<std::shared_ptr<Core::thread> > threads;
	for (std::size_t i = 0; i < Core::thread::hardware_concurrency() - 2; ++i) {
		threads.push_back( 
			std::make_shared<Core::thread>( 
				boost::bind( &boost::asio::io_service::run, 
					&(*io) 
				) 
			) 
		);
	}

	DwmClient client;
	threads.push_back( 
		std::make_shared<Core::thread>( 
			boost::bind( &DwmClient::run, &client ) 
		) 
	);

	size_t mainLoopThreadIndex = threads.size() - 1;

	Dwm server( client.getClientWorld() );
	threads.push_back( 
		std::make_shared<Core::thread>( 
			boost::bind( &Dwm::bootstrapLocal, &server )
		)
	);

/*		if( Handshake( *io, hostname, port ) == true ) 
	{
		// Wait for all threads in the pool to exit.
		for (std::size_t i = 0; i < threads.size(); ++i)
			threads[i]->join();
	}*/
	threads[ mainLoopThreadIndex ]->join();

	MMU::shutdown();
	BitCoder::shutdown();

	return 0;
}

void DWMMain( std::shared_ptr<Core::thread> leash ) {
	LOG(INFO) << "DWMMain running\n";

	// TODO get address and ports from remote
	// hardcoded from base hostname for now
	std::shared_ptr<Dwm> dwm = std::make_shared<Dwm>();
	dwm->openCommChans( io, hostname );
	dwm->bootstrapLocal();

	while( true ) {
		Core::this_thread::sleep( boost::posix_time::millisec(50) );
	}
}

