//!-----------------------------------------------------
//!
//! \file material.cpp
//! the material system
//! Link to semantics html in FX composer
//! file:///C:/Program%20Files/NVIDIA%20Corporation/NVIDIA%20FX%20Composer/Data/fxmapping.htm
//!
//!-----------------------------------------------------

#include "graphics_pch.h"
#include "graphics.h"
#include "vbmanager.h"
#include "ibmanager.h"
#include "wobfile.h"
#include "gfxresources.h"
#include "vdeclmanager.h"
#include "core/resourceman.h"
#include "rendercamera.h"
#include "rendermatrixcache.h"
#include "material.h"
#include "texture.h"
#include "lightcontext.h"
#include "rendercontext.h"

namespace Graphics {
uint32_t MaterialWob::highMaterialId = 0;

Material::Material() :
	m_ShaderHandle( 0 ) {
	numTextures = 0;
	for( uint32_t i = 0;i < MAX_TEXTURES;i++) {
		m_TextureHandles[i] = 0;
	}	
}

Material::~Material() {
	for( uint32_t i = 0;i < numTextures;i++)
	{
		if( m_TextureHandles[i] ) {
			Core::ResourceMan::Get()->CloseResource( m_TextureHandles[i] );
		}
	}
	Core::ResourceMan::Get()->CloseResource( m_ShaderHandle );
}

MaterialWob::MaterialWob() :
	m_uiVertexSize( 0 ) {
}

MaterialWob::~MaterialWob() {
	VBManager::Get()->FreeVertices( m_VertexBuffer );
	IBManager::Get()->FreeIndices( m_IndexBuffer );
}

void MaterialWob::CreateFromWobMaterial( WobMaterial& wobMaterial ) {
	origWobMaterial = &wobMaterial;
	materialId = highMaterialId++;

	m_ShaderHandle = ShaderFXHandle::Load( wobMaterial.pShader );
	
	m_uiVertexSize = VDeclManager::CalcVertexSize( wobMaterial.numVertexElements, wobMaterial.pElements );
	m_VertexBuffer = VBManager::Get()->AllocVertices( wobMaterial.pVertexData, wobMaterial.uiNumVertices * m_uiVertexSize );
	int indexSize = (wobMaterial.uiFlags & WobMaterial::WM_32BIT_INDICES)? sizeof(uint32_t) : sizeof(uint16_t);

	m_IndexBuffer = IBManager::Get()->AllocIndices( wobMaterial.pIndexData, wobMaterial.uiNumIndices * indexSize  );

	localAabb = Core::AABB( wobMaterial.minAABB, wobMaterial.maxAABB );

	ShaderFXPtr effect;

	uint32_t iParamBytes = 0;

	effect = m_ShaderHandle->Acquire();
	
	for( uint16_t i=0;i < wobMaterial.uiNumParameters;++i) {
		WobMaterialParameter* pParam = &wobMaterial.pParameters[i];
		if( pParam->uiType >= WobMaterialParameter::WMPT_SCALAR_FLOAT && pParam->uiType <= WobMaterialParameter::WMPT_VEC4_FLOAT ) {
			// TODO correct sets
			// we have to pass in 4 floats even tho our side and the effets side isn't...
			//oh well could write my own validaiton but meh i'm lazy...
			if( pParam->uiType == WobMaterialParameter::WMPT_SCALAR_FLOAT ) {
				effect->setVariable( pParam->pName, *((float*)pParam->pData) );
			} else {
				effect->setVariable( pParam->pName, (float*)pParam->pData );
			}
		} else
		if( pParam->uiType >= WobMaterialParameter::WMPT_TEXTURE_CUBE_MAP && pParam->uiType <= WobMaterialParameter::WMPT_TEXTURE_3D ) {
			const char* pTexName = (const char*)pParam->pData;
			m_TextureHandles[ numTextures ] = TextureHandle::Load( pTexName );
			effect->setVariable( pParam->pName, m_TextureHandles[ numTextures ] );
			numTextures++;
		} else {
			assert( false && "Unknown or not supported parameter type" );
		}
	}

	if( effect->isSemanticUsed( "MaterialId" ) ) {
		effect->setVariable( "MaterialId", materialId );
	}
}

void MaterialWob::Render( RenderContext* context, const Core::string& renderNamePass ) {
	ShaderFXPtr effect;

	if( false ) {
		effect = Gfx::Get()->GetSolidWireShaderHandle()->Acquire();
	} else {
		effect = m_ShaderHandle->Acquire();
	}

	if( m_VDeclHandles.find( renderNamePass ) == m_VDeclHandles.end() ) {
		m_VDeclHandles[renderNamePass] = VDeclManager::Get()->GetVertexDeclarationHandle( origWobMaterial->numVertexElements, origWobMaterial->pElements, m_ShaderHandle, renderNamePass );
	}

	bool hasOwnEffect = effect->hasGroup(renderNamePass);
	if( hasOwnEffect ) {
		effect->selectGroup( renderNamePass );
	} else {
		CORE_ASSERT( context->referenceShader != NULL );
		effect = context->referenceShader;
	}

	// TODO only PerDrawCall version of setGlobalParameters
	effect->setGlobalParameters( context->matrixCache );

	context->deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	VDeclManager::Get()->SetVertexDecleration( context, m_VDeclHandles[renderNamePass] );
	m_VertexBuffer.Bind( context, 0, m_uiVertexSize );
	m_IndexBuffer.Bind( context );

	D3DX11_TECHNIQUE_DESC techDesc;
	effect->m_pEffectTechnique->GetDesc( &techDesc );

	for( uint32_t i = 0;i < techDesc.Passes;++i) {
		effect->m_pEffectTechnique->GetPassByIndex( i )->Apply( 0,  context->deviceContext );
		context->deviceContext->DrawIndexed( m_IndexBuffer.GetTriangleCount()*3, m_IndexBuffer.GetStartIndex(), 0 );
	}
	Gfx::Get()->incPrimitiveCount( m_IndexBuffer.GetTriangleCount() );
	m_VertexBuffer.Unbind( context, 0 );
}

}