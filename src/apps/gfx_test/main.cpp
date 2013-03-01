#include "core/core.h"
#include "shell3d.h"
#include "scene/hier.h"
#include "localworld/sceneworld.h"
#include "localworld/thing.h"
#include "localworld/thingfactory.h"

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

	Shell3D shell;
	shell.start();
	SceneWorldPtr world = shell.getSceneWorld();

	Scene::HierPtr land = std::make_shared<Scene::Hier>( "basic" );
	ThingPtr tng( ThingFactory::createThingFromHier( land, TBC_WORLD ) );
	world->add( tng );
/*	Scene::HierPtr stinger = std::make_shared<Scene::Hier>( "stinger" );
	ThingPtr stingr = std::make_shared<Thing>( stinger );
//	world->add( stingr );
	Scene::HierPtr mechh = std::make_shared<Scene::Hier>( "ogremech" );
	ThingPtr mech = std::make_shared<Thing>( mechh );
	world->add( mech );*/																																							

	shell.run();
	shell.end();

//	auto gfxThread = std::make_shared<Core::thread>( 
//			boost::bind( &DwmClient::run, &client ) 
//		);

//	gfxThread->join();

	return 0;
}
