#include "core/core.h"
#include "shell3d.h"
#include "scene/hier.h"
#include "localworld/sceneworld.h"
#include "localworld/thing.h"
#include "localworld/thingfactory.h"
#include "gfxdbgconsole.h"
#include "voxtree.h"
#include "voxtreerenderable.h"

//#include "boost/program_options.hpp"



class PrintDebugPrims :	public Core::DebugRenderInterface {
	public:
	PrintDebugPrims( SceneWorldPtr world ) : 
		dbgConsole( CORE_NEW GfxDbgConsole( world ) ) {
			prevDRI = Core::g_pDebugRender;
			Core::g_pDebugRender = this;
	}

	virtual void ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) override { prevDRI->ndcLine( colour, a, b ); }
	virtual void worldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b ) override { prevDRI->worldLine( colour, a, b ); }
	virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& pos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override { prevDRI->worldSphere( colour, pos, radius, transform ); }
	virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override { prevDRI->worldCylinder( colour, pos, orient, radius, height, transform ); }
	virtual void worldBox( const Core::Colour& colour, const Math::Vector3& pos, const Math::Quaternion& orient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override { prevDRI->worldBox( colour, pos, orient, lx, ly, lz, transform ); }

	virtual void print( const char* pText ) override {
		dbgConsole->print( pText );
	}
	
	virtual int getNumberOfVarPrints() const override {
		return dbgConsole->VAR_COUNT;
	}

	virtual void varPrint( const int _index, const Math::Vector2& _pos, const char* _text ) override {
		dbgConsole->printVar( _index, _pos.x, _pos.y, _text );
	}

private:
	std::unique_ptr<GfxDbgConsole>	dbgConsole;
	DebugRenderInterface*			prevDRI;					//!< the previous Debug Render Interface we delegate all non print calls too
};



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
	std::unique_ptr<PrintDebugPrims> pdp( CORE_NEW PrintDebugPrims(world) );

	extern Vox::Tree* VoxTreeTest();
	Vox::Tree* tree = VoxTreeTest();
	Vox::TreeRenderablePtr treeRender = std::make_shared<Vox::TreeRenderable>( *tree );
	ThingPtr thing ( ThingFactory::createEmptyThing( TBC_WORLD, NewThingId() ) );
	thing->add( treeRender );
	world->add( thing );

/*	// some basic setup
	Scene::HierPtr land = std::make_shared<Scene::Hier>( "test_room" );
	ThingPtr tng( ThingFactory::createThingFromHier( land, TBC_WORLD ) );
	world->add( tng );
*/
	shell.run(); // will loop until exit is called

	// cleanup
//	world->remove( tng );
//	tng.reset();
//	land.reset();

	// shutdown
	pdp.reset();
	world.reset();
	shell.end();

//	auto gfxThread = std::make_shared<Core::thread>( 
//			boost::bind( &DwmClient::run, &client ) 
//		);

//	gfxThread->join();

	return 0;
}
