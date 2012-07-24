#include "dwm.h"
#include "trustedregion.h"
#include "apis/apis.h"

void World::InstallApiFuncs( TrustedRegion* trustedRegion ) {
	InstallDebugApiFuncs( trustedRegion );
	InstallVmApiFuncs( trustedRegion );
	InstallResourcesApiFuncs( trustedRegion );	
	InstallHierApiFuncs( trustedRegion );	
}
