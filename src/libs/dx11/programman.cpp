//!-----------------------------------------------------
//!
//! \file programman.cpp
//! manager for the shaders
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "program.h"
#include "programman.h"
#include <d3d11shader.h>
#include <D3Dcompiler.h>

namespace {
	const char* compilerTypeStrings[ Scene::MAX_SHADER_TYPES ] = { "vs_5_0", "ps_5_0", "gs_5_0", "ds_5_0", "hs_5_0", "cs_5_0" };
}

namespace Dx11
{

void* ProgramMan::compileShader( Scene::SHADER_TYPES type, const std::string& src ) {
	ID3DBlob* codeBlob;
	ID3DBlob* errBlob;
	HRESULT hr;
	UINT flags;
IF_DEBUG_START
	flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_DEBUG;
IF_DEBUG_ELSE
	flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
IF_DEBUG_END

	hr = D3DCompile( src.c_str(), src.size(), nullptr, nullptr, nullptr, "main", 
			compilerTypeStrings[ type ], flags, 0, &codeBlob, &errBlob );
	if( hr == S_OK ) {
		ID3D11ShaderReflection* reflector;
		D3DReflect( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector );
		return CORE_NEW std::pair<ID3DBlob*, ID3D11ShaderReflection*>(codeBlob, reflector);
	} else {
		LOG(FATAL) << "Shader Compile failed : " << src << "\n" << (const char*) errBlob->GetBufferPointer() << "\n";
		return nullptr;
	}
}
// todo stream out + class linkage
Program* ProgramMan::linkProgram( const Program::CreationInfo* creation ) {
	using namespace Scene;
	HRESULT hr;
	Program* prg = CORE_NEW Program();

	for( int i = 0; i < MAX_SHADER_TYPES; ++i ) {
		const auto data = (std::pair<ID3DBlob*, ID3D11ShaderReflection*>*)( creation->data[i] );
		if( data == nullptr ) {
			continue;
		}
		ID3DBlob* codeBlob = data->first;
		prg->reflector[ i ] = D3DShaderReflectionPtr( data->second, false );
		switch( i ) {
		case Scene::ST_FRAGMENT: {
				ID3D11PixelShader* shadr;
				DXFAIL( Gfx::getr()()->CreatePixelShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
			} break;
		case Scene::ST_VERTEX: {
				ID3D11VertexShader* shadr;
				DXFAIL( Gfx::getr()()->CreateVertexShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
			} break;
		case Scene::ST_GEOMETRY: {
				ID3D11GeometryShader* shadr;
				DXFAIL( Gfx::getr()()->CreateGeometryShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
			} break;
		case Scene::ST_HULL: {
				ID3D11HullShader* shadr;
				DXFAIL( Gfx::getr()()->CreateHullShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
			} break;
		case Scene::ST_DOMAIN: {
				ID3D11DomainShader* shadr;
				DXFAIL( Gfx::getr()()->CreateDomainShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
			} break;
		case Scene::ST_COMPUTE: {
				ID3D11ComputeShader* shadr;
				DXFAIL( Gfx::getr()()->CreateComputeShader( codeBlob->GetBufferPointer(), codeBlob->GetBufferSize(), nullptr, &shadr ) );
				prg->shader[ i ] = D3DDeviceChildPtr( shadr, false );
				prg->reflector[i]->GetThreadGroupSize( &prg->threadGroupXSize, &prg->threadGroupYSize, &prg->threadGroupZSize );
			} break;
		default: CORE_ASSERT( false );
		};	
		prg->src[i] = codeBlob;
	}
	return prg;
}

}

