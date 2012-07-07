#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "apis.h"

void DbgStringOut( const IEEThreadContext* threadCtx, UNTRUSTED_UINTPTR_T unText ) { 
	const char* text = (const char*)UNTRUSTED_PTR_TO_TRUSTED( unText );
	LOG(INFO) << text; 
};

void DbgIntOut( const IEEThreadContext* threadCtx, int32_t num ) { 
	LOG(INFO) << num; 
};


void InstallDebugApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "DbgStringOut", (void*) DbgStringOut );
	trustedRegion->addFunctionTrampoline( "DbgIntOut", (void*) DbgIntOut );
}