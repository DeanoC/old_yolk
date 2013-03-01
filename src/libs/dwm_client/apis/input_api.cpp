#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "dwm/isolatedexecengine.h"
#include "dwm/apis/apis.h"
#include "dwm_client/clientworld.h"

namespace Untrusted {
#include "../../../vt/src/libs/area/apis/input_api.h"
}

bool InputGetFrame( _VT_PARAMS1( _VT_PTR( Untrusted::InputFrame*, unFrame ) ) ) {
//	CORE_STATIC_ASSERT( sizeof(Untrusted::InputFrame) == sizeof(InputFrame) );

	InputFrame* frame = (InputFrame*)UNTRUSTED_PTR_TO_TRUSTED( unFrame );
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;

	return clientWorld->dequeueInputFrame( frame );
}


void InstallInputApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "InputGetFrame", (void*) InputGetFrame );
}
