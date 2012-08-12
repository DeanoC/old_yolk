///-------------------------------------------------------------------------------------------------
/// \file	graphics\lightcontextd3d11.cpp
///
/// \brief	Implements the lightcontext d3d11 class.
///
/// \details	
///		lightcontextd3d11 description goes here
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@rattie.demon.co.uk
///
/// \todo	Fill in detailed file description.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "graphics_pch.h"
#include "rendercontext.h"
#include "rendermatrixcache.h"
#include "lightcontext.h"

namespace {
struct GpuLight {
	float worldToLightSpace[16];
	float colour[4];
};

// must match gpu tbuffer exactly!
struct LightInfoTBuffer {
	int32_t numLights;
	int32_t dummy[3];
	GpuLight lights[10];
};
}
namespace Graphics {

LightContextD3D11::LightContextD3D11() {
	const int bufSize = sizeof(LightInfoTBuffer);
	assert( bufSize == 204*4 ); // manual check

	tbufferHandle = TextureHandle::Create( "_LightContext0", 
		&Texture::CreationStruct( 
			Texture::CF_BUFFER | Texture::CF_CPU_WRITE, // flags
			bufSize / (DXGIFormat::GetBitWidth(DXGI_FORMAT_R32G32B32A32_UINT) / 8), 0, DXGI_FORMAT_R32G32B32A32_UINT, 0 ) );
	tbuffer = tbufferHandle->Acquire();
}

LightContextD3D11::~LightContextD3D11() {
	tbufferHandle->Close();
}

int LightContextD3D11::getNumLights() const {
	return lights.size();
}

Light& LightContextD3D11::getLight( unsigned int iIndex ) {
	return *lights[iIndex];
}

const Light& LightContextD3D11::getLight( unsigned int iIndex ) const {
	return *lights[iIndex];
}

int LightContextD3D11::addLight( const LightPtr light ) {
	lights.push_back( light );
	return (int)lights.size()-1;
}

void LightContextD3D11::removeLight( int iIndex ) { 
	lights.erase( lights.begin() + iIndex);
}

void LightContextD3D11::updateGPU( RenderContext *const context ) {
	// lock the current tbuffer
	uint32_t outPitch;
	void* buf = tbuffer->Lock( context, Texture::CA_WRITE_DISCARD, outPitch );
	
	// set up header
	*(((uint32_t*)buf)+0) = getNumLights();
	float* data = ((float*)buf);

	// transfer lights across
	for( int i = 0, x = 4; i < getNumLights(); ++i ) {
		const Light& l = getLight(i);
		l.writeToGpuBuffer(&data[x]);
		x += l.getGpuSize()/sizeof(float);
	}

	tbuffer->Unlock( context );
}

}
