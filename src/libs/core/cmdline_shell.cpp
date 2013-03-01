// 
//  cmdline_shell.cpp
//  deano
//  
// 

#include "core.h"

extern int Main();

extern "C" int main( int argc, char** argv ) {
#if defined( USE_GLOG )
	google::InitGoogleLogging(argv[0]);
#endif
	Core::Init();

	Core::g_argc = argc;
	if( Core::g_argc > Core::MAX_CMDLINE_ARGS ) {
		Core::g_argc = Core::MAX_CMDLINE_ARGS;
	}
	memcpy( Core::g_argv, argv, sizeof(char*) * Core::g_argc );
	int res = 0;
	CoreTry {
		res = Main();
	}
	CoreCatchAllOurExceptions {
		LogException( err );
		LOG(FATAL) << "Fatal Exception: \n";
	}
/*	CoreCatchAllStdExceptions {
		if( err.what() ) {
			LOG(FATAL) << err.what();
		} else {
			LOG(FATAL) << "std exception caught, but has no what\n";
		}
	}
	CoreCatchAll {
		LOG(FATAL) << "Fatal Unknown exception: \n";
	}*/

	Core::Shutdown();
#if defined( USE_GLOG )
	google::ShutdownGoogleLogging();
#endif
	return res;
}
