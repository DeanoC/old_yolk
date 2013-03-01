/**
 @file	program.cpp

 @brief	Implements the programs objects.
 */
#include "dx11.h"
#include <d3d11shader.h>
#include <D3Dcompiler.h>
#include "program.h"

namespace Dx11 {
	bool Program::usesConstantBuffer( const Scene::SHADER_TYPES type, const char* bufferName ) const { 
		if( reflector[type] ) {
			const auto& refl = reflector[type];
			if( refl->GetConstantBufferByName( bufferName ) != nullptr ) {
				return true;
			}
		}
		return false;
	}

	bool Program::usesConstantBuffer( const char* bufferName) const { 
		for( int i = 0; i < Scene::MAX_SHADER_TYPES;++i ) {	
			if( usesConstantBuffer( (Scene::SHADER_TYPES) i, bufferName ) ) {
				return true;
			}
		}
		return false;
	}

	uint32_t Program::getVariableOffset( const char* bufferName, const char* name ) const { 
		HRESULT hr;
		for( int i = 0; i < Scene::MAX_SHADER_TYPES;++i ) {	
			if( reflector[i] ) {
				const auto& refl = reflector[i];
				D3D11_SHADER_DESC sdesc;
				hr = refl->GetDesc( &sdesc );
				auto cbufrefl = refl->GetConstantBufferByName( bufferName );
				if( cbufrefl != nullptr ) {
					D3D11_SHADER_BUFFER_DESC cbdesc;
					hr = cbufrefl->GetDesc( &cbdesc );
					auto varrefl = cbufrefl->GetVariableByName( name );
					CORE_ASSERT( varrefl != nullptr ); // docs say never happens we get a sentinal
					D3D11_SHADER_VARIABLE_DESC desc;
					hr = varrefl->GetDesc( &desc );
					if( hr == S_OK && (desc.uFlags & D3D_SVF_USED) ) {
						return (uint32_t)desc.StartOffset;
					}
				}
			}
		}

		return ~0;
	}
}
