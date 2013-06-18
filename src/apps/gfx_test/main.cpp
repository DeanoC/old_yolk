#include "core/core.h"
#include "shell3d.h"
#include "scene/hier.h"
#include "localworld/sceneworld.h"
#include "localworld/thing.h"
#include "localworld/thingfactory.h"

//#include "boost/program_options.hpp"


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

	// set shell parameters here

	// start the shell and grab the world
	shell.start();
	SceneWorldPtr world = shell.getSceneWorld();

	extern void VoxTreeTest();
	VoxTreeTest();

	// some basic setup
	Scene::HierPtr land = std::make_shared<Scene::Hier>( "test_room" );
	ThingPtr tng( ThingFactory::createThingFromHier( land, TBC_WORLD ) );
	world->add( tng );

	shell.run(); // will loop until exit is called

	// cleanup
	world->remove( tng );
	tng.reset();
	land.reset();

	// shutdown
	world.reset();
	shell.end();

//	auto gfxThread = std::make_shared<Core::thread>( 
//			boost::bind( &DwmClient::run, &client ) 
//		);

//	gfxThread->join();

	return 0;
}
