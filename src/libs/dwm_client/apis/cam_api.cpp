#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "dwm/isolatedexecengine.h"
#include "dwm/apis/apis.h"
#include "dwm_client/clientworld.h"

#include "../../../vt/src/libs/area/apis/cam_api.h"


CamHandle CamLoad( _VT_PARAMS1( _VT_PTR( const char*, fname ) ) ) {
	LOG(INFO) << "CamLoad not supported yet\n";
	const char* lfname = (const char*)UNTRUSTED_PTR_TO_TRUSTED( fname );

	auto cam = std::make_shared<Scene::Camera>();
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	uint32_t handle = clientWorld->addCamera( cam );
	return (CamHandle) handle;
}

CamHandle CamNew( _VT_PARAMS0() ) {
	auto cam = std::make_shared<Scene::Camera>();
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	uint32_t handle = clientWorld->addCamera( cam );
	return (CamHandle) handle;
}

void CamDelete( _VT_PARAMS1( CamHandle handle ) ) {
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	clientWorld->removeCamera( handle );
}

void CamSetFOV( _VT_PARAMS3( CamHandle handle, float fov, float aspect ) ) {
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	auto cam = clientWorld->getCamera( handle );
	cam->setFOV( fov * Math::degree_to_radian<float>(), aspect );
}

void CamSetDepthRange( _VT_PARAMS3( CamHandle handle, float near, float far ) ) {
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	auto cam = clientWorld->getCamera( handle );
	cam->setDepthRange( near, far );
}

void CamSetView( _VT_PARAMS2( CamHandle handle, _VT_PTR( const float*, unMatrix ) ) ) {
	_VT_YOLK_PTR( const float*, unMatrix, matrix );
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	auto cam = clientWorld->getCamera( handle );
	cam->setView( Math::Matrix4x4( matrix ) );
}


void CamSetActive( _VT_PARAMS1( CamHandle handle ) ) {
	ClientWorld* clientWorld = (ClientWorld*) threadCtx->owner->world;
	auto cam = clientWorld->getCamera( handle );

	return clientWorld->setActiveCamera( cam );
}

void InstallCamApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "CamLoad", (void*) CamLoad );
	trustedRegion->addFunctionTrampoline( "CamNew", (void*) CamNew );
	trustedRegion->addFunctionTrampoline( "CamDelete", (void*) CamDelete );

	trustedRegion->addFunctionTrampoline( "CamSetFOV", (void*) CamSetFOV );
	trustedRegion->addFunctionTrampoline( "CamSetDepthRange", (void*) CamSetDepthRange );
	trustedRegion->addFunctionTrampoline( "CamSetView", (void*) CamSetView );
	trustedRegion->addFunctionTrampoline( "CamSetActive", (void*) CamSetActive );
}
