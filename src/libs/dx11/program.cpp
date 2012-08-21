/**
 @file	program.cpp

 @brief	Implements the programs objects.
 */
#include "dx11.h"
#include <d3d11shader.h>
#include <D3Dcompiler.h>
#include "program.h"

namespace Dx11 {
	bool Program::usesConstantBuffer( const Scene::SHADER_TYPES type, const uint32_t bufferIndex ) { 
		if( reflector[type] ) {
			const auto& refl = reflector[type];
			if( refl->GetConstantBufferByIndex( bufferIndex ) != nullptr ) {
				return true;
			}
		}
		return false;
	}

	bool Program::usesConstantBuffer( const uint32_t bufferIndex) { 
		for( int i = 0; i < Scene::MAX_SHADER_TYPES;++i ) {	
			if( usesConstantBuffer( (Scene::SHADER_TYPES) i, bufferIndex ) ) {
				return true;
			}
		}
		return false;
	}

	uint32_t Program::getVariableOffset( const uint32_t bufferIndex, const char* name ) { 
		for( int i = 0; i < Scene::MAX_SHADER_TYPES;++i ) {	
			if( reflector[i] ) {
				const auto& refl = reflector[i];
				auto cbufrefl = refl->GetConstantBufferByIndex( bufferIndex );
				if( cbufrefl != nullptr ) {
					auto varrefl = cbufrefl->GetVariableByName( name );
					CORE_ASSERT( varrefl != nullptr ); // docs say never happens we get a sentinal
					D3D11_SHADER_VARIABLE_DESC desc;
					HRESULT hr = varrefl->GetDesc( &desc );
					if( hr == S_OK ) {
						return (uint32_t)desc.StartOffset;
					}
				}
			}
		}

		return ~0;
	}
}
