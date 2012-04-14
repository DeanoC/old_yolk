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

	int res = Main();

	Core::Shutdown();
#if defined( USE_GLOG )
	google::ShutdownGoogleLogging();
#endif
	return res;
}
