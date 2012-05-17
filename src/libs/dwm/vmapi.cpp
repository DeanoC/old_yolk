#include "dwm.h"

#include "mmu.h"
#include "sandboxmemorymanager.h"
#include "trustedregion.h"
#include "ieethreadcontext.h"
#include "isolatedexecengine.h"

static uint32_t VmAlloc( const IEEThreadContext* threadCtx, uint32_t size ) {
	uintptr_t tAddr = (uintptr_t) threadCtx->owner->sandboxAllocate( size );
	return (uint32_t)(tAddr - threadCtx->membase);
}
static void VmFree( const IEEThreadContext* threadCtx, uintptr_t sAddr ) {
	threadCtx->owner->sandboxFree( (void*)(sAddr + threadCtx->membase) );	
}

void InstallVmApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "VmAlloc", (void*) VmAlloc );
	trustedRegion->addFunctionTrampoline( "VmFree", (void*) VmFree );
}
