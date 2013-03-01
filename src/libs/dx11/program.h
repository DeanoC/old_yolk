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

		virtual bool usesConstantBuffer( const Scene::SHADER_TYPES type, const char* bufferName ) const override; 
		virtual bool usesConstantBuffer( const char* bufferName ) const override;	
		virtual uint32_t getVariableOffset( const char* bufferName, const char* name ) const override;

	protected:
		D3DDeviceChildPtr			shader[ Scene::MAX_SHADER_TYPES ];
		D3DBlobPtr					src[ Scene::MAX_SHADER_TYPES ];
		D3DShaderReflectionPtr		reflector[ Scene::MAX_SHADER_TYPES ];
		// these three are specific to the compute shader, and allows displatch to be in threads and the shader
		// determine how big each group should be
		uint32_t					threadGroupXSize;
		uint32_t					threadGroupYSize;
		uint32_t					threadGroupZSize;
	};
}

#endif
