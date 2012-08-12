//!-----------------------------------------------------
//!
//! \file shader.h
//! the engine graphics shader class
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_SHADER_H_ )
#define YOLK_DX11_SHADER_H_
#pragma once

#include "core/resources.h"
#include "core/resourceman.h"
#include "texture.h"
#include "dxeffects11/Inc/d3dx11effect.h"

// forward decl
struct ID3DX11Effect;
struct ID3DX11EffectVariable;

namespace Dx11
{	
	// forward decl
	class RenderMatrixCache;

	//! Shader FX Type
	static const uint32_t StaticShaderType = RESOURCE_NAME('S','S','H','R');
	static const uint32_t ShaderFXType = RESOURCE_NAME('S','H','D','R');

	//! StaticShader resource
	class StaticShader : public Core::Resource<StaticShaderType> {
	public:
		struct CreationStruct {};
		struct LoadStruct {};

		StaticShader() : m_pEffect(0) {}
		~StaticShader(){ SAFE_RELEASE(m_pEffect); }

		ID3DX11Effect*				m_pEffect;			//!< the effect itself
	};
	typedef Core::AsyncResourceHandle<StaticShaderType,StaticShader>				StaticShaderHandle;
	typedef const public Core::AsyncResourceHandle<StaticShaderType,StaticShader>*	StaticShaderHandlePtr;
	typedef Core::shared_ptr<StaticShader>											StaticShaderPtr;


	//! Shader FX resource
	class ShaderFX : public Core::Resource<ShaderFXType> {
	public:
		struct CreationStruct {};
		struct LoadStruct {};

		typedef ID3DX11EffectVariable* SemanticHandle;
		typedef Core::unordered_map<Core::string, SemanticHandle> SemanticMap;

		enum SEMANTIC_TYPE {
			ST_UNKNOWN,
			ST_SCALAR_FLOAT,
			ST_VECTOR_FLOAT,
			ST_SCALAR_INT,
			ST_VECTOR_INT,
			ST_SCALAR_BOOL,
			ST_VECTOR_BOOL,
			ST_STRUCT,
			ST_TEXTURE,
		};

		ShaderFX();
		~ShaderFX();

		//! register all the semantics in the effect into the map (called automatically on creation)
		void registerAllSemantics();

		//! gets the handle from a string
		SemanticHandle getSemantic( const Core::string& semantic ) const {
			assert( isSemanticUsed( semantic) );
			Core::string sem_copy(semantic);
			std::transform( sem_copy.begin(), sem_copy.end(), sem_copy.begin(), tolower );

			return (*m_ParameterMap.find( sem_copy )).second;
		}

		void setSemantic( const Core::string& semantic, SemanticHandle handle ) {
			Core::string sem_copy(semantic);
			std::transform( sem_copy.begin(), sem_copy.end(), sem_copy.begin(), tolower );

			m_ParameterMap[ sem_copy ] = handle;
		}
		void setVariable( SemanticHandle param, float* vec4 );
		void setVariable( SemanticHandle param, const Math::Vector3& vec3 );
		void setVariable( SemanticHandle param, const Math::Vector4& vec4 );
		void setVariable( SemanticHandle param, const Math::Matrix4x4& matrix );
		void setVariable( SemanticHandle param, float scalar );
		void setVariable( SemanticHandle param, int scalar );
		void setVariable( SemanticHandle param, unsigned int scalar );
		void setVariable( SemanticHandle param, Graphics::TextureHandlePtr texHandlePtr );
		void setVariable( SemanticHandle param, Graphics::TexturePtr texPtr );
		void setVariableFromExtra( SemanticHandle param, Graphics::TextureHandlePtr texHandlePtr );
		void setVariableFromExtra( SemanticHandle param, Graphics::TexturePtr texPtr );
		void setVariable( SemanticHandle param, ID3D11ShaderResourceView* shaderView );
		void setUAV( SemanticHandle param, ID3D11UnorderedAccessView* uavView );
		void setUAV( SemanticHandle param, Graphics::TextureHandlePtr texHandlePtr );
		void setUAV( SemanticHandle param, Graphics::TexturePtr texPtr );
		void setVariableVectorArray( SemanticHandle param, int index, int count, float* vec4s );
		void setVariableArray( SemanticHandle param, int index, int count, float* scalar );
		void setVariableMatrixArray( SemanticHandle param, int index, int count, float* matrixs );

		void setUAV( const char* param, Graphics::TextureHandlePtr texHandlePtr );
		void setUAV( const char* param, Graphics::TexturePtr texPtr );
		void setUAV( const char* param, ID3D11UnorderedAccessView* uavView );
		void setVariableVectorArray( const char* param, int index, int count, float* vec4s );
		void setVariableArray( const char* param, int index, int count, float* scalar );
		void setVariableMatrixArray( const char* param, int index, int count, float* matrixs );
		template<typename T> void setVariable( const char* param, T data ) {
			ID3DX11EffectVariable* var = m_pEffect->GetVariableByName( param );
			setVariable( var, data );
		}

		ID3DX11EffectConstantBuffer* getConstantBuffer( const char* param );
		void setBuffer( const char* param, Graphics::TextureHandlePtr tex );
		void setBuffer( const char* param, Graphics::TexturePtr tex );

		//! returns true if this shader uses this semantic (only valid after RegisterAllSemantics)
		bool isSemanticUsed( const Core::string& semantic ) const;

		//! returns the type of the semantic (will assert if the semantic doesn't exist)
		SEMANTIC_TYPE getSemanticType( const Core::string& semantic ) const;

		void selectGroup( const Core::string& group );
		bool hasGroup( const Core::string& group ) const;

		//! sets the global SAS parameters that this shader uses (matrices etc.)
		void setGlobalParameters( RenderMatrixCache* cache );

		const StaticShaderHandle*		m_staticShaderHandle;
		ID3DX11Effect*				m_pEffect;			//!< the effect itself
		ID3DX11EffectTechnique*		m_pEffectTechnique;	//!< the technique were using
		SemanticMap					m_ParameterMap;		//!< the map betweem strings and handles

		typedef Core::unordered_map< Core::string, ID3DX11EffectTechnique* > Group2TechniqueMap;
		Group2TechniqueMap			m_groups;

	protected:
		uint64_t			m_GlobalBits;		//!< this tells us which global SAS parameter this shader uses
		SemanticHandle		m_GlobalHandle[64];	//!< we support upto 64 global SAS parameters
		HRESULT hr;

	};

	typedef Core::AsyncResourceHandle<ShaderFXType,ShaderFX, void*, Core::RMRF_DONTCACHE>	ShaderFXHandle;
	typedef const ShaderFXHandle*												ShaderFXHandlePtr;
	typedef Core::shared_ptr<ShaderFX>											ShaderFXPtr;

}

#endif