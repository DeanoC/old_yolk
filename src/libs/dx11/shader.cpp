//!-----------------------------------------------------
//!
//! \file shader.cpp
//! function for shaders
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "shadermanager.h"
#include "shader.h"
#include "rendermatrixcache.h"
#include "rendercamera.h"
#include "lightcontext.h"

namespace 
{
	const char* s_GlobalParametersNames[] = 
	{
		"View",
		"ViewInverse",
		"ViewInverseTranspose",
		"Projection",
		"ProjectionInverse",
		"ProjectionInverseTranspose",
		"ViewProjection",
		"ViewProjectionInverse",
		"ViewProjectionInverseTranspose",
		"World",
		"WorldInverse",
		"WorldInverseTranspose",
		"WorldView",
		"WorldViewInverse",
		"WorldViewInverseTranspose",
		"WorldViewProjection",
		"WorldViewProjectionInverse",
		"WorldViewProjectionInverseTranspose",
		"PrevWorldViewProjection",

		// Sas bind addresses
		"Sas.Skeleton.MeshToJointToWorld[0]",
		"Sas.Skeleton.MeshToJointToWorld[*]",
		"Sas.Skeleton.NumJoints",

		"Sas.Camera.WorldToView",
		"Sas.Camera.Projection",

		"Sas.Time",

		"Sas.EnvironmentMap",

		"Sas.Shadow[].WorldToShadow",  
		"Sas.Shadow[].ShadowMap",   
		"Sas.NumShadows",

		"DepthTargetTexture",
		"OldDepthTargetTexture",
		"OldResultTargetTexture",
		"ScreenRandomTexture",
		"Normal_Encode",
		"Normal_Decode",
		"MRT0",
		"MRT1",
		"MRT2",
		"MRT3",
		"MRT4",
		"MRT5",
		"MRT6",
		"MRT7",

		"FrameCount",
		"ScreenDimension",
		"ScreenDimensionInQuads",
		"DepthDecode",
		"PrimitiveIdStart"
	};
}

namespace Dx11
{

ShaderFX::ShaderFX() :
	m_pEffect(0),
	m_GlobalBits(0) {
}

//! dtor
ShaderFX::~ShaderFX() {
	m_staticShaderHandle->Close();
	SAFE_RELEASE( m_pEffect );
}

//! register all the semantics in the effect into the map (called automatically on creation)
void ShaderFX::registerAllSemantics() {
	assert( m_pEffect );
	HRESULT hr;

	// we now need to parse the effect and build a list of parameters. We use 
	// MS SAS to allow our shaders to be used in DCC packages and other tools
	D3DX11_EFFECT_DESC effectDesc;
	DXWARN( m_pEffect->GetDesc( &effectDesc ) );

	for( unsigned int i=0; i <effectDesc.GlobalVariables; ++i ) {
		ID3DX11EffectVariable* globVar = m_pEffect->GetVariableByIndex( i );
		if( globVar == 0 )
			continue;

		// we could search the 
		D3DX11_EFFECT_VARIABLE_DESC paramDesc;
		DXWARN( globVar->GetDesc( &paramDesc ) );

		// note potentially there is a conflict if something has a SasBindAddress and a semantic...

		// if the parameter has a sementic register it
		if( paramDesc.Semantic != 0 ) {
			setSemantic( paramDesc.Semantic, globVar );
		}
		// we tread SasBindAddress as semantics, so we have to search all the 
		for( UINT i=0;i < paramDesc.Annotations;++i) {
			ID3DX11EffectVariable* annotVar  = globVar->GetAnnotationByIndex(i);
			D3DX11_EFFECT_VARIABLE_DESC annotDesc;
			DXWARN( annotVar->GetDesc( &annotDesc ) );
			if( strcmp( annotDesc.Name, "SasBindAddress" ) == 0 ) {
				if( annotVar->AsString() ) {
					const char* str;
					annotVar->AsString()->GetString( &str );
					setSemantic( str, globVar );
				}
			}
		}
	}

	static const int iNumGlobals = sizeof(s_GlobalParametersNames) / sizeof(s_GlobalParametersNames[0]);

	// this could be a compile time check but I don't care at the mo...
	assert( iNumGlobals < 64 );

	// now see which global SAS parameters this shader uses

	for( unsigned int i=0;i < iNumGlobals ;++i) {
		if( isSemanticUsed( s_GlobalParametersNames[i] ) == true ) {
			m_GlobalBits |= (1i64 << i);
			m_GlobalHandle[i] = getSemantic( s_GlobalParametersNames[i] );
		}
	}

	// does this shader want camera data
	if( isSemanticUsed( "Sas.Camera" ) == true ) {
	}

}

bool ShaderFX::isSemanticUsed( const Core::string& semantic ) const {
	Core::string sem_copy(semantic);
	Core::transform( sem_copy.begin(), sem_copy.end(), sem_copy.begin(), tolower );
	return( m_ParameterMap.find( sem_copy ) != m_ParameterMap.end() );
}

ShaderFX::SEMANTIC_TYPE ShaderFX::getSemanticType( const Core::string& semantic ) const {
	HRESULT hr;
	SemanticHandle hSemantic = getSemantic( semantic );

	D3DX11_EFFECT_TYPE_DESC typeDesc;
	DXWARN( hSemantic->GetType()->GetDesc( &typeDesc ) );
	switch( typeDesc.Type )
	{
	case D3D_SVT_BOOL:
		if( typeDesc.Class == D3D_SVC_SCALAR )
			return ST_SCALAR_BOOL;
		if( typeDesc.Class == D3D_SVC_VECTOR )
			return ST_VECTOR_BOOL;
		break;
	case D3D_SVT_INT:
		if( typeDesc.Class == D3D_SVC_SCALAR )
			return ST_SCALAR_INT;
		if( typeDesc.Class == D3D_SVC_VECTOR )
			return ST_VECTOR_INT;
		break;
	case D3D_SVT_FLOAT:
		if( typeDesc.Class == D3D_SVC_SCALAR )
			return ST_SCALAR_FLOAT;
		if( typeDesc.Class == D3D_SVC_VECTOR )
			return ST_VECTOR_FLOAT;
		break;
	case D3D_SVT_TEXTURE:
	case D3D_SVT_TEXTURE1D:
	case D3D_SVT_TEXTURE2D:
	case D3D_SVT_TEXTURE3D:
	case D3D_SVT_TEXTURECUBE:
		return ST_TEXTURE;
	default:
		break;
	};

	return ST_UNKNOWN;
}
void ShaderFX::setGlobalParameters( RenderMatrixCache* cache ) {
	HRESULT hr;
	unsigned int iBit = 0;
	// first up check which matrices this shader wants sets
	for( ;iBit < RenderMatrixCache::NUM_MATRICES;++iBit ) {
		if( m_GlobalBits & (1i64 << iBit) ) {
			DXWARN( m_GlobalHandle[iBit]->AsMatrix()->SetMatrix( (float*)&cache->getMatrix((RenderMatrixCache::MATRIX_TYPE) iBit) ) );
		}
	}

	// "Sas.Skeleton.MeshToJointToWorld[0]" AKA World matrix
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsMatrix()->SetMatrix( (float*)&cache->getMatrix( RenderMatrixCache::WORLD ) ) );
	}
	++iBit;

	// "Sas.Skeleton.MeshToJointToWorld[*]"
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsMatrix()->SetMatrixArray( (float*)cache->getBoneMatrixArray(), 0, cache->MAX_BONE_MATRICES ) );
	}
	++iBit;

	// "Sas.Skeleton.NumJoints",
	if( m_GlobalBits & (1i64 << iBit) ) {
		// currently always set to maximum number of joints this is probably wrong...
		DXWARN( m_GlobalHandle[iBit]->AsScalar()->SetInt( cache->MAX_BONE_MATRICES ) );

	}
	++iBit;

	// "Sas.Camera.WorldToView"
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsMatrix()->SetMatrix( (float*)&cache->getMatrix( RenderMatrixCache::VIEW ) ) );
	}
	++iBit;

	// "Sas.Camera.Projection"
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsMatrix()->SetMatrix( (float*)&cache->getMatrix( RenderMatrixCache::PROJECTION ) ) );
	}
	++iBit;

	// Sas bind addresses todo

	// "Sas.Time"
	++iBit;

	// "Sas.EnvironmentMap",
	++iBit;

	//"Sas.Shadow[].WorldToShadow",  
	++iBit;
	//"Sas.Shadow[].ShadowMap",   
	++iBit;
	//"Sas.NumShadows"
	++iBit;

	// "DepthTargetTexture"
	unsigned int firstGlobalTexture = iBit;	
	for( ;iBit < firstGlobalTexture+Gfx::MAX_GLOBAL_TEXTURES;++iBit ) {
		if( m_GlobalBits & (1i64 << iBit) ) {
			TextureHandlePtr texHandlePtr = Gfx::Get()->getGlobalTexture( (Gfx::GLOBAL_TEXTURES) (iBit - firstGlobalTexture) );
			TexturePtr texResource = texHandlePtr->Acquire();
			m_GlobalHandle[iBit]->AsShaderResource()->SetResource( (ID3D11ShaderResourceView*) texResource->m_primaryView );
		}
	}

	// FrameCount
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsScalar()->SetInt( Gfx::Get()->getFrameCount() ) );
	}
	++iBit;

	// ScreenDimension
	if( m_GlobalBits & (1i64 << iBit) ) {
		float fscdim[] = {	(float) Gfx::Get()->getScreenWidth(), 
							(float) Gfx::Get()->getScreenHeight(), 
							1.0f / (float) Gfx::Get()->getScreenWidth(), 
							1.0f / (float) Gfx::Get()->getScreenHeight() };
		DXWARN( m_GlobalHandle[iBit]->AsVector()->SetFloatVector( fscdim ) );
	}
	++iBit;

	// ScreenDimensionInQuad
	if( m_GlobalBits & (1i64 << iBit) ) {
		float fscdim[] = { (float) (Gfx::Get()->getScreenWidth() / 2), (float) (Gfx::Get()->getScreenHeight() / 2), 0, 0 };
		DXWARN( m_GlobalHandle[iBit]->AsVector()->SetFloatVector( fscdim ) );
	}
	++iBit;

	// DepthDecode
	if( m_GlobalBits & (1i64 << iBit) ) {
		RenderCameraPtr camera = cache->getCamera();
		float a = camera->getZFar() / ( camera->getZFar() - camera->getZNear() );
		float b = (-camera->getZFar() * camera->getZNear()) / (camera->getZFar() - camera->getZNear() );
		float fscdim[] = { camera->getZNear(), camera->getZFar(), a, b };
		DXWARN( m_GlobalHandle[iBit]->AsVector()->SetFloatVector( fscdim ) );
	}
	++iBit;

	// PrimitiveIdStart
	if( m_GlobalBits & (1i64 << iBit) ) {
		DXWARN( m_GlobalHandle[iBit]->AsScalar()->SetInt( Gfx::Get()->getPassPrimitiveCount() ) );
	}
	++iBit;
}

void ShaderFX::setVariable( SemanticHandle param, float* vec4 ) {
	DXWARN( param->AsVector()->SetFloatVector( vec4 ) );
}
void ShaderFX::setVariable( SemanticHandle param, const Math::Vector4& vec4 ) {
	DXWARN( param->AsVector()->SetFloatVector( (float*)&vec4 ) );
}
void ShaderFX::setVariable( SemanticHandle param, const Math::Vector3& vec3 ) {
	DXWARN( param->AsVector()->SetFloatVector( (float*)&vec3 ) );
}
void ShaderFX::setVariable( SemanticHandle param, const Math::Matrix4x4& matrix ) {
	DXWARN( param->AsMatrix()->SetMatrix( (float*)&matrix) );
}
void ShaderFX::setVariable( SemanticHandle param, float scalar ) {
	DXWARN( param->AsScalar()->SetFloat( scalar ) );
}
void ShaderFX::setVariable( SemanticHandle param, int scalar ) {
	DXWARN( param->AsScalar()->SetInt( scalar ) );
}
void ShaderFX::setVariable( SemanticHandle param, unsigned int scalar ) {
	DXWARN( param->AsScalar()->SetRawValue( &scalar,0, 4 ) );
}

void ShaderFX::setVariable( SemanticHandle param, ID3D11ShaderResourceView* shaderView ) {
	DXWARN( param->AsShaderResource()->SetResource( shaderView ) );
}

void ShaderFX::setVariable( SemanticHandle param, TextureHandlePtr texHandlePtr ) {
	TexturePtr texResource = texHandlePtr->Acquire();
	setVariable( param, texResource );
}
void ShaderFX::setVariable( SemanticHandle param, TexturePtr texPtr ) {
	DXWARN( param->AsShaderResource()->SetResource( (ID3D11ShaderResourceView*) texPtr->m_primaryView ) );
}
void ShaderFX::setVariableFromExtra( SemanticHandle param, TextureHandlePtr texHandlePtr ) {
	TexturePtr texResource = texHandlePtr->Acquire();
	setVariableFromExtra(param, texResource);
}
void ShaderFX::setVariableFromExtra( SemanticHandle param, TexturePtr texPtr ) {
	DXWARN( param->AsShaderResource()->SetResource( (ID3D11ShaderResourceView*) texPtr->m_extraView ) );
}

void ShaderFX::setUAV( SemanticHandle param, ID3D11UnorderedAccessView* uavView ) {
	DXWARN( param->AsUnorderedAccessView()->SetUnorderedAccessView( uavView ) );
}
void ShaderFX::setUAV( SemanticHandle param, TextureHandlePtr texHandlePtr ) {
	TexturePtr texResource = texHandlePtr->Acquire();
	setUAV( param, texResource );
}
void ShaderFX::setUAV( SemanticHandle param, TexturePtr texPtr ) {
	DXWARN( param->AsUnorderedAccessView()->SetUnorderedAccessView( (ID3D11UnorderedAccessView*) texPtr->m_extraView ) );
}

void ShaderFX::setVariableVectorArray( SemanticHandle param, int index, int count, float* vec4s ) {
	DXWARN( param->AsVector()->SetFloatVectorArray( vec4s, index, count ) );
}
void ShaderFX::setVariableMatrixArray( SemanticHandle param, int index, int count, float* matrixs ) {
	DXWARN( param->AsMatrix()->SetMatrixArray( matrixs, index, count ) );
}

void ShaderFX::setVariableArray( SemanticHandle param, int index, int count, float* scalars ) {
	DXWARN( param->AsScalar()->SetFloatArray( scalars, index, count ) );
}

void ShaderFX::setVariableVectorArray( const char* param, int index, int count, float* vec4s ) {
	ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
	setVariableVectorArray( var, index, count, vec4s );
}
void ShaderFX::setVariableMatrixArray( const char* param, int index, int count, float* matrixs ) {
	ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
	setVariableMatrixArray( var, index, count, matrixs );
}

void ShaderFX::setVariableArray( const char* param, int index, int count, float* scalars ) {
	ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
	setVariableArray(var, index, count, scalars );
}

void ShaderFX::setUAV( const char* param, TextureHandlePtr texHandlePtr ) {
	TexturePtr texResource = texHandlePtr->Acquire();
	setUAV( param, texResource );
}
void ShaderFX::setUAV( const char* param, TexturePtr texPtr ) {
	ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
	setUAV( var, texPtr );
}
void ShaderFX::setUAV( const char* param, ID3D11UnorderedAccessView* uavView ) {
	ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
	setUAV( var, uavView );
}

void ShaderFX::selectGroup( const Core::string& group ) {
	if( m_groups.find( group ) != m_groups.end() ) {
		m_pEffectTechnique = m_groups[ group ];
	} else {
		// group is not available if technique hasn't been set use first, else leave alone
		if( m_pEffectTechnique == NULL ) {
			m_pEffectTechnique = (*m_groups.begin()).second;
		}
	}
}

bool ShaderFX::hasGroup( const Core::string& group ) const {
	if( m_groups.find( group ) != m_groups.end() ) {
		return true;
	} else {
		return false;
	}
}

ID3DX11EffectConstantBuffer* ShaderFX::getConstantBuffer(  const char* param ) {
	return 0;
}

void ShaderFX::setBuffer( const char* param, TextureHandlePtr texHandlePtr ) {
	HRESULT hr;
	TexturePtr texResource = texHandlePtr->Acquire();

	ID3DX11EffectVariable* globVar = m_pEffect->GetConstantBufferByName( param );
	DXWARN( globVar->AsConstantBuffer()->SetTextureBuffer( (ID3D11ShaderResourceView*) texResource->m_primaryView ) );
}
void ShaderFX::setBuffer( const char* param, TexturePtr texPtr ) {
	HRESULT hr;
	ID3DX11EffectVariable* globVar = m_pEffect->GetConstantBufferByName( param );
	DXWARN( globVar->AsConstantBuffer()->SetTextureBuffer( (ID3D11ShaderResourceView*) texPtr->m_primaryView ) );
}

}