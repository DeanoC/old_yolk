#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "dwm/isolatedexecengine.h"
#include "dwm/trustedregion.h"
#include "dwm/world.h"

#include "core/resourceman.h"
#include "core/coreresources.h"
#include "scene/hie.h"

#include "apis.h"

#include "../../../vt/src/libs/area/resources_api.h"
#include "../../../vt/src/libs/area/hier_api.h"

//--------------
// creation 
//--------------

HierHandle HierCreateMesh( _VT_PARAMS2( MeshType type, _VT_PTR( const MeshCreateParams* const, params ) ) ) {
	using namespace Core;
	MeshCreateParams safeParams;
	memcpy( &safeParams, (void*) UNTRUSTED_PTR_TO_TRUSTED(params), sizeof( MeshCreateParams ) );

	switch( type ) {
		case MeshType::Sphere: {
			auto sphere = std::make_shared<Scene::Hie>( "sphere_1" );
			sphere->getTransformNode()->setLocalScale( Math::Vector3(safeParams.radius, safeParams.radius, safeParams.radius) );
			uint32_t handle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) sphere.get() );
			threadCtx->owner->world->addRenderable( sphere );
			return (HierHandle) handle;
		}
		break;
		case MeshType::Box:
		default:
			LOG(INFO) << "ResCreateMesh for a type of " << (void*)type << " is not currently reported";
			return 0;			
	}

	return 0;
}

HierHandle HierLoadHier( _VT_PARAMS1( _VT_PTR( const char*, fname ) ) ) {
	const char* lfname = (const char*)UNTRUSTED_PTR_TO_TRUSTED( fname );
	auto mesh = std::make_shared<Scene::Hie>( lfname );
	uint32_t handle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) mesh.get() );
	threadCtx->owner->world->addRenderable( mesh );
	return (HierHandle) handle;
}

//--------------
// general
//--------------

// returns RES_ERROR if has no environment
ResHandle HierOpenEnvironmentBinProperty( _VT_PARAMS1( HierHandle handle ) )  {
	auto hie = (Scene::Hie*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( (uintptr_t) handle );
	if( hie->getEnvironment() ) {
		auto phandle = hie->getEnvironment()->getPropertiesResourceHandle();
		uint32_t propHandle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) phandle );
		return (ResHandle) propHandle;
	} else {
		return RES_ERROR;
	}
}

//--------------
// node
//--------------

int HierGetNodeCount( _VT_PARAMS1( HierHandle handle ) ) {
	auto hie = (Scene::Hie*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( (uintptr_t) handle );
	return hie->getNodeCount();
}

void HierSetLocalTransform( _VT_PARAMS3( HierHandle handle, _VT_PTR( const HierTransformParams* const, params ), int nodeId ) ) {
	HierTransformParams safeParams;
	memcpy( &safeParams, (void*) UNTRUSTED_PTR_TO_TRUSTED(params), sizeof( HierTransformParams ) );

	auto hie = (Scene::Hie*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( (uintptr_t) handle );

	hie->getTransformNode( nodeId )->setLocalPosition( Math::Vector3(	safeParams.localPos[0], 
	                                          					safeParams.localPos[1], 
	                                          					safeParams.localPos[2]) );
	hie->getTransformNode( nodeId )->setLocalOrientation( Math::Quaternion(	safeParams.localQuat[0], 
	                                             					safeParams.localQuat[1], 
	                                             					safeParams.localQuat[2], 
	                                             					safeParams.localQuat[3]) );
}

void HierGetLocalTransform( _VT_PARAMS3( HierHandle handle, _VT_PTR( const HierTransformParams*, params ), int nodeId ) ) {

	HierTransformParams safeParams;

	auto hie = (Scene::Hie*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( (uintptr_t) handle );

	auto pos = hie->getTransformNode( nodeId )->getLocalPosition();
	auto rot = hie->getTransformNode( nodeId )->getLocalOrientation();

	safeParams.localPos[0] = pos.x;
	safeParams.localPos[1] = pos.y;
	safeParams.localPos[2] = pos.z;

	safeParams.localQuat[0] = rot.x;
	safeParams.localQuat[1] = rot.y;
	safeParams.localQuat[2] = rot.z;
	safeParams.localQuat[3] = rot.w;

	memcpy( (void*) UNTRUSTED_PTR_TO_TRUSTED(params), &safeParams, sizeof( HierTransformParams ) );

}

// returns RES_ERROR if has no environment
ResHandle HierOpenNodeBinProperty( _VT_PARAMS2( HierHandle handle, int nodeId ) )  {
	auto hie = (Scene::Hie*) threadCtx->owner->getTrustedRegion()->handleToTrustedAddress( (uintptr_t) handle );
	if( !hie->getProperties( nodeId ) ) {
		return RES_ERROR;
	}

	auto pandle = hie->getProperties( nodeId )->clone();
	uint32_t propHandle = threadCtx->owner->getTrustedRegion()->trustedAddressToHandle( (uintptr_t) pandle );
	return (ResHandle) propHandle;
}

void InstallHierApiFuncs( TrustedRegion* trustedRegion ) {

	trustedRegion->addFunctionTrampoline( "HierCreateMesh", (void*) HierCreateMesh );
	trustedRegion->addFunctionTrampoline( "HierLoadHier", (void*) HierLoadHier );

	trustedRegion->addFunctionTrampoline( "HierOpenEnvironmentBinProperty", (void*) HierOpenEnvironmentBinProperty );

	trustedRegion->addFunctionTrampoline( "HierGetNodeCount", (void*) HierGetNodeCount );
	trustedRegion->addFunctionTrampoline( "HierSetLocalTransform", (void*) HierSetLocalTransform );
	trustedRegion->addFunctionTrampoline( "HierGetLocalTransform", (void*) HierGetLocalTransform );
	trustedRegion->addFunctionTrampoline( "HierOpenNodeBinProperty", (void*) HierOpenNodeBinProperty );

}