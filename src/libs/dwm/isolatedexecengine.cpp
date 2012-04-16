#include "dwm.h"
#include "isolatedexecengine.h"

IsolatedExecEngine::IsolatedExecEngine( llvm::Module* init ) {
	using namespace llvm;

	EngineBuilder builder( init );
	// TODO enable JIT
	builder.setEngineKind( EngineKind::Interpreter ); 
	exec.reset( builder.create() );

}