//!-----------------------------------------------------
//!
//! \file resourceloader.cpp
//! async loader/creator handler + thread
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "scene/wobfile.h"
#include "scene/hierfile.h"
#include "scene/textureatlas.h"
#include "texture.h"
#include "program.h"
#include "databuffer.h"
#include "vertexinput.h"
#include "renderstates.h"
#include "programman.h"

#include "resourceloader.h"

namespace Dx11 {

Scene::DataBuffer* ResourceLoader::createDataBuffer( const void* data ) {
	auto creation = (const Scene::DataBuffer::CreationInfo*) data;
	return DataBuffer::internalCreate( creation );
}
Scene::Texture* ResourceLoader::createTexture( const void* data ) {
	auto creation = (const Scene::Texture::CreationInfo*) data;
	return Texture::internalCreate( creation );
}

Scene::VertexInput* ResourceLoader::createVertexInput( const void* data ) {
	auto creation = (const Scene::VertexInput::CreationInfo*) data;
	return VertexInput::internalCreate( creation );
}

Scene::SamplerState* ResourceLoader::createSamplerState( const void* data ) {
	auto creation = (const Scene::SamplerState::CreationInfo*) data;
	return SamplerState::internalCreate( creation );
}
Scene::RenderTargetStates* ResourceLoader::createRenderTargetStates( const void* data ) {
	auto creation = (const Scene::RenderTargetStates::CreationInfo*) data;
	return RenderTargetStates::internalCreate( creation );
}
Scene::DepthStencilState* ResourceLoader::createDepthStencilState( const void* data ) {
	auto creation = (const Scene::DepthStencilState::CreationInfo*) data;
	return DepthStencilState::internalCreate( creation );
}
Scene::RasteriserState* ResourceLoader::createRasteriserState( const void* data ) {
	auto creation = (const Scene::RasteriserState::CreationInfo*) data;
	return RasteriserState::internalCreate( creation );
}

}
