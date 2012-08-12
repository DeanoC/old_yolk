//!-----------------------------------------------------
//!
//! \file shadermanager.cpp
//! manager for the shaders
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "shadermanager.h"
#include "core/resourceman.h"
#include "core/coreresources.h"
#include "core/file_path.h"

namespace {
	#include "sasfxh.h"
} // end anon namespace

// Note Shader Includer isn't yet thread safe, so instances are needed for each compile, note this makes the map
// largely useless as not perserved across different files... but hopefully for now windows files cache will be 
// enough
struct ShaderIncluder : public ID3DInclude
{
	typedef Core::map< Core::string, Core::TextResourceHandlePtr > HandleMap;

	STDMETHOD(Open)(THIS_	D3D_INCLUDE_TYPE IncludeType, 
							LPCSTR pFileName, 
							LPCVOID pParentData, 
							LPCVOID *ppData, 
							UINT *pBytes ) {
		// special case sas
		if( 0 == lstrcmpiA( "sas/Sas.fxh", pFileName)
			|| 0 == lstrcmpiA( "sas\\Sas.fxh", pFileName)
			|| 0 == lstrcmpiA( "Sas.fxh", pFileName) ) {
			*pBytes= lstrlenA(s_SasFxh);
			*ppData = CORE_NEW char[ *pBytes ];
			memcpy( (void*)(*ppData), s_SasFxh, *pBytes );

			return S_OK;
		}
		char pTemp[2048];

		if( 0 != strncmp( pFileName,"Shaders", 7 ) ) {
			strncpy( pTemp, "Shaders/", 2048 );
			pFileName = strncat(pTemp, pFileName, 2048 );
		}

		Core::TextResourceHandlePtr handle( m_TextHandleMap[ pFileName ] );
		if( handle == 0 ) {
			handle = m_TextHandleMap[ pFileName ] = 
				Core::TextResourceHandle::Load( pFileName );
		} else {
			if( m_bCacheText == false ) {
				Core::TextResourceHandle::Flush( pFileName );
				handle->Close();
				handle = m_TextHandleMap[ pFileName ] = Core::TextResourceHandle::Load( pFileName );
			}
		}

		Core::shared_ptr<Core::TextResource>	pTextResource;

		pTextResource = handle->Acquire();

		// note if your text file is over 4 GB!!! this will be incorrect
		*pBytes = (UINT)strlen(pTextResource->m_saText.get());
		*ppData = CORE_NEW_ARRAY char[ *pBytes ];
		memcpy( (void*)(*ppData), pTextResource->m_saText.get(), *pBytes );

		return S_OK;
	}

	STDMETHOD(Close)(THIS_ LPCVOID pData) {
		CORE_DELETE pData;
		return S_OK;
	}

	// cache text off because not thread safe
	ShaderIncluder() : 
		m_bCacheText( false ) {
	}

	~ShaderIncluder() {
		for( HandleMap::iterator rIt = m_TextHandleMap.begin();
			rIt != m_TextHandleMap.end();
			++rIt ) {
			Core::ResourceMan::Get()->CloseResource( rIt->second );
		}
	}

	HandleMap	m_TextHandleMap;	//!< list of filename we have already loaded
	bool		m_bCacheText;		//!< 
};


namespace Dx11
{
ShaderManager::ShaderManager() {

	D3D_SHADER_MACRO tailShaderMacro;
	tailShaderMacro.Name = 0;
	tailShaderMacro.Definition = 0;
	d3dXShaderMacros.push_back( tailShaderMacro );

#if DEBUG_LEVEL >= DEBUG_NONE
	D3D_SHADER_MACRO debugShaderMacro;
	debugShaderMacro.Name = "_DEBUG";
	debugShaderMacro.Definition = "1";

	d3dXShaderMacros.insert( d3dXShaderMacros.begin(), debugShaderMacro );
#endif

	m_pIncluder = CORE_NEW ShaderIncluder();
}

ShaderManager::~ShaderManager() {
	CORE_DELETE m_pIncluder;
}

ShaderFX* ShaderManager::InternalLoadShaderFX( const Core::ResourceHandleBase* baseHandle, const char* pFilename, bool preLoad ) {
	const StaticShaderHandle* handle = StaticShaderHandle::Load( pFilename, NULL, preLoad ? Core::RMRF_PRELOAD : Core::RMRF_NONE );

	ShaderFX* pResource = CORE_NEW ShaderFX();
	pResource->m_staticShaderHandle = handle;
	Core::shared_ptr<StaticShader>	staticShader = handle->Acquire();
	CORE_ASSERT( staticShader->m_pEffect != NULL );
	staticShader->m_pEffect->CloneEffect(0, &pResource->m_pEffect );

	D3DX11_EFFECT_DESC fxDesc;
	pResource->m_pEffect->GetDesc( &fxDesc );

	for( unsigned int i=0;i < fxDesc.Groups;i++ ) {
		ID3DX11EffectGroup* group = pResource->m_pEffect->GetGroupByIndex( i );
		if ( group != NULL ) {
			D3DX11_GROUP_DESC groupDesc;
			group->GetDesc( &groupDesc );

			bool hasTechnique = false;
			Gfx::SHADER_MODEL sm = Gfx::Get()->getShaderModel();
			while( hasTechnique == false ) {
				const Core::string& hwSm = Gfx::Get()->getShaderModelAsString( sm );
				for( unsigned int j=0;j < groupDesc.Techniques;j++ ) {
					ID3DX11EffectTechnique* tech = group->GetTechniqueByIndex( j );
					D3DX11_TECHNIQUE_DESC techDesc;
					tech->GetDesc( &techDesc );
					if( hwSm == techDesc.Name ) {
						pResource->m_groups[ groupDesc.Name ?  groupDesc.Name : "" ]  = tech;
						hasTechnique = true;
						break;
					}
				}
				if( sm > Gfx::SM4_0 ) {
					sm = (Gfx::SHADER_MODEL) ((int)sm-1);
				} else {
					// this group has no valid technique so will be missing from the shader group list
					hasTechnique = true;
				}
			}
		}
	}

	// make sure we have at least one valid group
	CORE_ASSERT( pResource->m_groups.size() > 0 );


	pResource->m_pEffectTechnique = NULL;
	pResource->selectGroup( "" ); // default to global ungroup technique
	CORE_ASSERT( pResource->m_pEffectTechnique );
	CORE_ASSERT( pResource->m_pEffectTechnique->IsValid() );
	pResource->registerAllSemantics();

	return pResource;

}

StaticShader* ShaderManager::InternalLoadStaticShader( const Core::ResourceHandleBase* baseHandle, const char* pFilename, bool preLoad ) {
	Core::FilePath path( pFilename );
	path = path.ReplaceExtension( ".fx" );
	const char* pShaderFileName = path.value().c_str();

	const StaticShaderHandle* handle = (const StaticShaderHandle*) baseHandle; 

	ID3DBlob *shaderBlob;
	ID3DBlob *errorBlob;
	HRESULT hr;
	if( FAILED( handle->asyncResult ) && handle->asyncResultPtr == NULL ) {
		// shader has started compiling but hasn't finished so wait
		while( handle->asyncResult == E_PENDING && handle->asyncResultPtr == NULL ) {
			DXFAIL( Graphics::Gfx::Get()->GetThreadPump()->ProcessDeviceWorkItems( 1 ) );
		}
		if( handle->asyncResult != E_PENDING || handle->asyncResultPtr == NULL ) {
			if( FAILED( handle->asyncResult ) ) {
				DXWARN( handle->asyncResult );
			}
		
			// if shader compiling fails or is try again every 5 seconds,
			// should let you fix it you fool!
			while( FAILED(handle->asyncResult) ) {
				DXWARN( D3DX11CompileFromFile(	pShaderFileName,
												&d3dXShaderMacros[0],
												m_pIncluder,
												NULL,
												"fx_5_0",
												D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
												0,
												NULL,
												(ID3DBlob**)&handle->asyncResultPtr,
												&errorBlob,
												NULL ) );
				handle->asyncResult = hr;
				if( FAILED(hr) ) {
					LOG(INFO) << "Shader compile failed, retry in 10 seconds\n";
					if( errorBlob != NULL ) {
						LOG(INFO) << (char*) errorBlob->GetBufferPointer();
					}
					Sleep(10000);
				}
			}
		}
	} else if(handle->asyncResultPtr == NULL) {
		if( handle->asyncResult != E_PENDING ) {
			// just doing a preload so kick one off
			handle->asyncResultPtr = 0;
			handle->asyncResult = E_PENDING;

			D3DX11CompileFromFile(	pShaderFileName,
									&d3dXShaderMacros[0],
									m_pIncluder,
									NULL,
									"fx_5_0",
									D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS,
									0,
									Gfx::Get()->GetThreadPump(),
									(ID3DBlob**)&handle->asyncResultPtr,
									NULL,
									&handle->asyncResult );
		}
		return NULL;
	} 

	// here we have to finish the load (TODO async this)
	StaticShader* pResource = CORE_NEW StaticShader();
	shaderBlob = (ID3DBlob*)handle->asyncResultPtr;
	DXWARN( D3DX11CreateEffectFromMemory( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, Gfx::Get()->GetDevice(), & pResource->m_pEffect ) );
	shaderBlob->Release();

	//
	return pResource;
}

}