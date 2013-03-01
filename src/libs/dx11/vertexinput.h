//!-----------------------------------------------------
//!
//! \file texture.h
//! the engine graphics texture class
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_VERTEXINPUT_H_ )
#define YOLK_DX11_VERTEXINPUT_H_
#pragma once

#if !defined( YOLK_SCENE_VERTEXINPUT_H_ )
#include "scene/vertexinput.h"
#endif
#if !defined( YOLK_DX11_DATABUFFER_H_ )
#include "databuffer.h"
#endif

namespace Dx11 {
	class VertexInput : public Scene::VertexInput {
	public:
		friend class RenderContext;
		virtual void validate( Scene::ProgramPtr program ) override;

		static Scene::VertexInput* internalCreate( const Scene::VertexInput::CreationInfo* creation );

		VertexInput() {}

	protected:
		int							streamCount;
		size_t						streamStrides[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
		Scene::DataBufferHandlePtr	streamBuffers[ Scene::VertexInput::MAX_ELEMENT_COUNT ];	
		int							elementCount;
		D3D11_INPUT_ELEMENT_DESC 	vertexElements[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
		D3DInputLayoutPtr			inputLayout;

	};
}

#endif