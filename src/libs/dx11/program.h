#pragma once

//!-----------------------------------------------------
//!
//! \file program.h
//! A Program is a resource made of a set of GPU shaders
//! for the different parts of the GPU
//!-----------------------------------------------------
#if !defined( YOLK_DX11_PROGRAM_H_ )
#define YOLK_DX11_PROGRAM_H_

#include "scene/program.h"

namespace Dx11 {
	class Program : public Scene::Program {
	public:
		friend class ProgramMan;
		friend class VertexInput;
		friend class RenderContext;

		virtual bool usesConstantBuffer( const Scene::SHADER_TYPES type, const uint32_t bufferIndex ) override; 
		virtual bool usesConstantBuffer( const uint32_t bufferIndex ) override;	
		virtual uint32_t getVariableOffset( const uint32_t bufferIndex, const char* name ) override;

	protected:
		D3DDeviceChildPtr			shader[ Scene::MAX_SHADER_TYPES ];
		D3DBlobPtr					src[ Scene::MAX_SHADER_TYPES ];
		D3DShaderReflectionPtr		reflector[ Scene::MAX_SHADER_TYPES ];
	};
}

#endif
