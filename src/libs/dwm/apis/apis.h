#pragma once
#ifndef _YOLK_DWM_APIS_APIS_H_
#define _YOLK_DWM_APIS_APIS_H_ 1

class TrustedRegion;

// TODO debug versus release(SFI) type magic
#define UNTRUSTED_UINTPTR_T uint64_t
#define TRUSTED_PTR_TO_UNTRUSTED(x) (UNTRUSTED_UINTPTR_T)((x) - threadCtx->membase)
#define UNTRUSTED_PTR_TO_TRUSTED(x) ((uintptr_t)(x) + threadCtx->membase)

#define _VT_PARAMS0( ) const IEEThreadContext* threadCtx
#define _VT_PARAMS1( x ) const IEEThreadContext* threadCtx, x
#define _VT_PARAMS2( x, y ) const IEEThreadContext* threadCtx, x, y
#define _VT_PARAMS3( x, y, z ) const IEEThreadContext* threadCtx, x, y, z
#define _VT_PTR( x, y ) UNTRUSTED_UINTPTR_T y
#define _VT_YOLK_PTR( x, y, z ) x z = (x) UNTRUSTED_PTR_TO_TRUSTED(y)

void InstallDebugApiFuncs( TrustedRegion* trustedRegion );
void InstallVmApiFuncs( TrustedRegion* trustedRegion );
void InstallResourcesApiFuncs( TrustedRegion* trustedRegion );
void InstallHierApiFuncs( TrustedRegion* trustedRegion );

#endif