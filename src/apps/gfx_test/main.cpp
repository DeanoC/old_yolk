#include "core/core.h"
#include "dwm_client/client.h"
#include "scene/hie.h"

//#include "boost/program_options.hpp"
//#include "json_spirit/json_spirit_reader.h"
/*
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
   }
}
*/
#include "dwm/trustedregion.h"
void World::InstallApiFuncs( class TrustedRegion* trustedRegion ) {
}

uint8_t* TrustedRegion::encodeFunc( void* ) {
	return 0;
}

int Main() {
	using namespace Core;
/*	namespace po = boost::program_options;

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
	*/

	DwmClient client;
	client.start();
	ClientWorldPtr world = client.getClientWorld();

	Scene::HierPtr land = std::make_shared<Scene::Hier>( "dynamics_test1.hie" );
	world->addRenderable( land );
	client.run();
	client.end();

//	auto gfxThread = std::make_shared<Core::thread>( 
//			boost::bind( &DwmClient::run, &client ) 
//		);

//	gfxThread->join();

	return 0;
}
