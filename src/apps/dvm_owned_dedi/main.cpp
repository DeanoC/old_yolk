#include "core/core.h"
#include "gl/gl.h"
#include "cl/cl.h"
#include "gl/gfx.h"
#include "cl/platform.h"
#include "dwm/dwm.h"
#include "dwm/bitcoder.h"
#include "dwm/mmu.h"
#include "heart.h"
#include "boost/program_options.hpp"
#include "json_spirit/json_spirit_reader.h"
#include "handshake.h"
#include "core/coreresources.h"
#include "render/hieskeleton.h"
#include "render/camera.h"
#include "core/clock.h"
#include "core/sysmsg.h"
#include "core/development_context.h"
#include "debugcamcontext.h"

#define START_FULLSCREEN	false
#define START_WIDTH			1024
#define START_HEIGHT		1024
#define START_AA			AA_NONE
void MainLoop();

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

bool g_bQuitFlag = false;
void QuitCallback() {
	g_bQuitFlag = true;
}
int g_iDebugMode = 0;
void DebugModeCallback( int debugMode ) {
	g_iDebugMode = debugMode;
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
	CoreTry {
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
		for (std::size_t i = 0; i < Core::thread::hardware_concurrency(); ++i) {
			threads.push_back( 
				std::make_shared<Core::thread>( 
					boost::bind( &boost::asio::io_service::run, 
						&(*io) 
					) 
				) 
			);
		}

		Gl::Gfx::init();
		if( !Cl::Platform::exists() )
			Cl::Platform::init();

		Core::InitWindow( START_WIDTH, START_HEIGHT, START_FULLSCREEN );
		bool glOk = Gl::Gfx::get()->createScreen( START_WIDTH, START_HEIGHT, START_FULLSCREEN, Gl::Gfx::START_AA );
		if( glOk == false ) {
			LOG(ERROR) << "GL unable to find adequate GPU";
			return 1;
		}
		Core::SystemMessage::get()->registerQuitCallback( QuitCallback );
		Core::SystemMessage::get()->registerDebugModeChangeCallback( DebugModeCallback );

		Render::RenderContext* ctx = (Render::RenderContext*) Gl::Gfx::get()->getThreadRenderContext(0);
		Core::DevelopmentContext::get()->addContext( "DebugCam", 
				std::shared_ptr<DebugCamContext>( CORE_NEW DebugCamContext( ctx ) ) );
		Core::DevelopmentContext::get()->activateContext( "DebugCam" );

		MainLoop();

/*		Dwm test;
		test.bootstrapLocal();

		if( Handshake( *io, hostname, port ) == true ) {
			// Wait for all threads in the pool to exit.
			for (std::size_t i = 0; i < threads.size(); ++i)
				threads[i]->join();
		}*/
	} 
	CoreCatchAllOurExceptions {
		LogException( err );
		return 1;
	}
	CoreCatchAllStdExceptions {
		LOG(ERROR) << err.what();
		return 1;
	}
	CoreCatchAll {
		return 1;
	}

//	Graphics::ScrConsole::Shutdown();
//	Cl::Platform::Get()->destroyDevices();
	Gl::Gfx::get()->shutdownScreen();
//	Cl::Platform::Shutdown();
	Gl::Gfx::shutdown();

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

#include "render/renderworld.h"

void MainLoop() {
	using namespace Core;
	Render::RenderWorld rworld;

	ScopedAsyncResourceHandle<ManifestResourceHandle> sphMan( ManifestResourceHandle::load( "sphere_1" ) );
	auto sphere = CORE_NEW Render::HieSkeleton( "sphere_1" );
	rworld.addRenderable( sphere );

	Render::RenderContext* ctx = (Render::RenderContext*) Gl::Gfx::get()->getThreadRenderContext(0);

	// flush 'load' time from first time update
	Clock::get()->update();

	float x = 0.0f;
	// Main loop
	while( !g_bQuitFlag ) {

		float deltaT = Clock::get()->update();

		DevelopmentContext::get()->update( deltaT );

//		x += deltaT * 10;
		sphere->getTransformNode()->setLocalPosition( Math::Vector3(x, 0, 0) );

		rworld.render( ctx );
		rworld.debugDraw( ctx );

		DevelopmentContext::get()->display();
		Gl::Gfx::get()->present();
		Core::HouseKeep();
	}	
	CORE_DELETE sphere;
}