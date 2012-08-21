//!-----------------------------------------------------
//!
//! \file texture.cpp
//!
//!-----------------------------------------------------

#include "scene.h"
#include "renderstates.h"

namespace Scene {

// loading goes here, if we ever serialize render states

const void* SamplerState::internalPreCreate( const char* name, const SamplerState::CreationInfo *loader ) {
	return loader;
}

const void* RenderTargetStates::internalPreCreate( const char* name, const RenderTargetStates::CreationInfo *loader ) {
	return loader;
}

const void* DepthStencilState::internalPreCreate( const char* name, const DepthStencilState::CreationInfo *loader ) {
	return loader;
}

const void* RasteriserState::internalPreCreate( const char* name, const RasteriserState::CreationInfo *loader ) {
	return loader;
}

}