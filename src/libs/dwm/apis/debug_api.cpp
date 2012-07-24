#include "dwm/dwm.h"
#include "dwm/trustedregion.h"
#include "dwm/ieethreadcontext.h"
#include "apis.h"

#include "core/debug_render.h"

#include "../../../vt/src/libs/area/apis/dbg_api.h"

void DbgStringOut( _VT_PARAMS1( _VT_PTR( const char*, unText ) ) ) { 
	const char* text = (const char*)UNTRUSTED_PTR_TO_TRUSTED( unText );
	LOG(INFO) << text; 
};

void DbgIntOut( _VT_PARAMS1( int32_t num ) ) { 
	LOG(INFO) << num; 
};

//! draw a NDC (projection) space line (resolution independent)
void DbgNdcLine( _VT_PARAMS3( _VT_PTR( const float*, a ), _VT_PTR( const float*,b ), const uint32_t leCol ) ) {
	const float* sa = (const float*) UNTRUSTED_PTR_TO_TRUSTED( a );
	const float* sb = (const float*) UNTRUSTED_PTR_TO_TRUSTED( b );
	const Core::RGBAColour col = Core::RGBAColour::unpackARGB( leCol );

	Core::g_pDebugRender->ndcLine( col, Math::Vector2( sa ), Math::Vector2( sb) );

}
//! draw a 3D world space line
void DbgWorldLine( _VT_PARAMS3( _VT_PTR( const float*, a ), _VT_PTR( const float*, b ), const uint32_t leCol ) ) {

	const float* sa = (const float*) UNTRUSTED_PTR_TO_TRUSTED( a );
	const float* sb = (const float*) UNTRUSTED_PTR_TO_TRUSTED( b );
	const Core::RGBAColour col = Core::RGBAColour::unpackARGB( leCol );

	Core::g_pDebugRender->worldLine( col, Math::Vector3( sa ), Math::Vector3( sb) );

}

//! world sphere
void DbgWorldSphere( _VT_PARAMS3( _VT_PTR( const float*, pos ), const float radius, const uint32_t leCol ) ) {

	const float* spos = (const float*) UNTRUSTED_PTR_TO_TRUSTED( pos );
	const Core::RGBAColour col = Core::RGBAColour::unpackARGB( leCol );

	Core::g_pDebugRender->worldSphere( col, Math::Vector3( spos ), radius );
}
/*
//! print some text onto some form of screen console
void DbgPrint( const char* pText ) {
}

//! world cylinder, cylinders are along the z
void DbgWorldCylinder( const Core::Colour& colour, 
						const Math::Vector3& localPos, 
						const Math::Quaternion& localOrient, 
						const float radius, 
						const float height ) {

}
//! world box
void DbgWorldBox( 	const Core::Colour& colour, 
					const Math::Vector3& localPos, 
					const Math::Quaternion& localOrient, 
					const float lx, const float ly, const float lz ) {

}

void DbgWorldBox( const Core::Colour& colour, 	const Math::Vector3& localPos, 
												const Math::Quaternion& localOrient, 
												const Math::Vector3& len ){
	DbgWorldBox( colour, localPos, localOrient, len.x, len.y, len.z );
}

void DbgWorldRect( const Colour& colour, 	const Math::Vector3& a, 
											const Math::Vector3& b, 
											const Math::Vector3& c, 
											const Math::Vector3& d ) {
	DbgWorldLine( colour, a , b );
	DbgWorldLine( colour, b , c );
	DbgWorldLine( colour, c , d );
	DbgWorldLine( colour, d , a );
}
*/

void InstallDebugApiFuncs( TrustedRegion* trustedRegion ) {
	trustedRegion->addFunctionTrampoline( "DbgStringOut", (void*) DbgStringOut );
	trustedRegion->addFunctionTrampoline( "DbgIntOut", (void*) DbgIntOut );

	trustedRegion->addFunctionTrampoline( "DbgNdcLine", (void*) DbgNdcLine );
	trustedRegion->addFunctionTrampoline( "DbgWorldLine", (void*) DbgWorldLine );
	trustedRegion->addFunctionTrampoline( "DbgWorldSphere", (void*) DbgWorldSphere );
}
