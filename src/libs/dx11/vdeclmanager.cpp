//!-----------------------------------------------------
//!
//! \file material.cpp
//! the material system
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "gfx.h"
#include "vbmanager.h"
#include "ibmanager.h"
#include "wobfile.h"
#include "rendercontext.h"
#include "vdeclmanager.h"

namespace Dx11
{

ShaderFXHandlePtr VDeclManager::referenceShaderHandle;

VDeclManager::~VDeclManager() {
	VDeclMap::iterator vdIt = m_VDeclMap.begin();
	while( vdIt != m_VDeclMap.end() ) {
		CORE_DELETE (*vdIt).second.m_Type;
		(*vdIt).second.m_inputLayout->Release();
		++vdIt;
	}
}

uint32_t VDeclManager::GetVertexDeclarationHandle( int numElements, WobVertexElement *vtypes, const ShaderFXHandlePtr shaderHandle, const Core::string&  shaderGroup ) {

	ShaderFXPtr shader = shaderHandle->Acquire();

	uint32_t hash = (uint32_t)shader->m_pEffect;
	hash += std::hash_value(shaderGroup);
	uint32_t upper = 0;

	WobVertexElement* type = vtypes;

	for( int i=0; i < numElements; ++i ){
		// hash is based on ELF used alot in unix circles
		uint32_t val = (((uint32_t)type->uiUsage)<<16) + type->uiType;
		hash = (hash << 4) + val;
		upper = hash & 0xF0000000;
		if( upper != 0)
		{
			hash ^= (upper >> 24);
			hash &= ~upper;

		}

		++type;
	};
	hash = hash & 0x7FFFFFFF;

	// now we have a hash, lets see if we have encountered it before
	VDeclMap::iterator vdIt = m_VDeclMap.find( hash );
	if( vdIt != m_VDeclMap.end() )
	{
		// check for hash collision
		assert( numElements == vdIt->second.m_numElements );
		assert( WobVertexElement::Compare( numElements, vtypes, vdIt->second.m_Type ) );
		return hash;
	} else
	{
		HRESULT hr;
		D3D11_INPUT_ELEMENT_DESC vertexElements[ 32 ];
		WobVertexElement* ourtype = vtypes;
		D3D11_INPUT_ELEMENT_DESC* d3dtype = vertexElements;
		VDeclItem vdecl;
		vdecl.m_Type = CORE_NEW WobVertexElement[numElements];
		vdecl.m_numElements = numElements;
		assert( numElements < 32 );

		uint32_t offset = 0;
		for( int i=0; i < numElements; ++i ) {
			vdecl.m_Type[i] = *ourtype;

			uint32_t beginOffset = offset;

			switch( ourtype->uiUsage ) {
			case WobVertexElement::WVTU_POSITION:
				d3dtype->SemanticName = "POSITION";	d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_NORMAL:
				d3dtype->SemanticName = "NORMAL"; d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_BINORMAL:
				d3dtype->SemanticName = "BINORMAL";	d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_TANGENT:
				d3dtype->SemanticName = "TANGENT"; d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_TEXCOORD0:
				d3dtype->SemanticName = "TEXCOORD"; d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_TEXCOORD1:
				d3dtype->SemanticName = "TEXCOORD"; d3dtype->SemanticIndex = 1; break;
			case WobVertexElement::WVTU_TEXCOORD2:
				d3dtype->SemanticName = "TEXCOORD"; d3dtype->SemanticIndex = 2; break;
			case WobVertexElement::WVTU_TEXCOORD3:
				d3dtype->SemanticName = "TEXCOORD"; d3dtype->SemanticIndex = 3; break;
			case WobVertexElement::WVTU_COLOUR0:
				d3dtype->SemanticName = "COLOR"; d3dtype->SemanticIndex = 0; break;
			case WobVertexElement::WVTU_COLOUR1:
				d3dtype->SemanticName = "COLOR"; d3dtype->SemanticIndex = 1; break;
			default:
				assert( false && "Unknown vertex usage" );
			}

			switch( ourtype->uiType )
			{
			case WobVertexElement::WVTT_FLOAT4:
				d3dtype->Format = DXGI_FORMAT_R32G32B32A32_FLOAT; offset += sizeof(float)*4; break;
			case WobVertexElement::WVTT_FLOAT3:
				d3dtype->Format = DXGI_FORMAT_R32G32B32_FLOAT; offset += sizeof(float)*3; break;
			case WobVertexElement::WVTT_FLOAT2:
				d3dtype->Format = DXGI_FORMAT_R32G32_FLOAT; offset += sizeof(float)*2; break;
			case WobVertexElement::WVTT_FLOAT1:
				d3dtype->Format = DXGI_FORMAT_R32_FLOAT; offset += sizeof(float); break;
			case WobVertexElement::WVTT_BYTEARGB:
				d3dtype->Format = DXGI_FORMAT_R8G8B8A8_UNORM; offset += sizeof(uint32_t); break;
			case WobVertexElement::WVTT_SHORT2:
				d3dtype->Format = DXGI_FORMAT_R16G16_UINT; offset += sizeof(short)*2; break;
			default:
				assert( false && "Inknown vertex type" );
			}

			d3dtype->InputSlot = 0;
			d3dtype->AlignedByteOffset = (UINT) beginOffset;
			d3dtype->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			d3dtype->InstanceDataStepRate = 0;

			++d3dtype;
			++ourtype;
		}

		vdecl.m_uiVertexSize = offset;
		if( shader->hasGroup( shaderGroup) ) {
			shader->selectGroup( shaderGroup );
		} else {
			shader = referenceShaderHandle->Acquire();
		}

		// Traverse all passes
		uint32_t pIndex = 0;
		ID3DX11EffectPass* pass = NULL;
		while((pass = shader->m_pEffectTechnique->GetPassByIndex(pIndex++))->IsValid()) {
			D3DX11_PASS_SHADER_DESC vs_desc;
			DXFAIL(pass->GetVertexShaderDesc(&vs_desc));

			D3DX11_EFFECT_SHADER_DESC s_desc;
			DXFAIL(vs_desc.pShaderVariable->GetShaderDesc(0, &s_desc));

			DXFAIL( Gfx::Get()->GetDevice()->CreateInputLayout( vertexElements, numElements, 
														s_desc.pBytecode, s_desc.BytecodeLength, &vdecl.m_inputLayout ) );
			break;// use first valid pass to create the input layout
		}

		m_VDeclMap[ hash ] = vdecl;

		return hash;
	}
}

void VDeclManager::SetVertexDecleration( RenderContext* context, uint32_t handle )
{
	assert( m_VDeclMap.find( handle) != m_VDeclMap.end() );
	context->deviceContext->IASetInputLayout( m_VDeclMap[ handle ].m_inputLayout );
}

uint32_t VDeclManager::GetVertexSize( uint32_t handle )
{
	assert( m_VDeclMap.find( handle) != m_VDeclMap.end() );
	return m_VDeclMap[ handle ].m_uiVertexSize;
}
void VDeclManager::SetReferenceShader( const ShaderFXHandlePtr shaderHandle ) {
	referenceShaderHandle = shaderHandle;
}

uint32_t VDeclManager::CalcVertexSize( int numElements, WobVertexElement *type ) {
	uint32_t offset = 0;
	for( int i=0; i < numElements; ++i ) {
		switch( type->uiType )
		{
		case WobVertexElement::WVTT_FLOAT4:   offset += sizeof(float)*4; break;
		case WobVertexElement::WVTT_FLOAT3:   offset += sizeof(float)*3; break;
		case WobVertexElement::WVTT_FLOAT2:   offset += sizeof(float)*2; break;
		case WobVertexElement::WVTT_FLOAT1:   offset += sizeof(float); break;
		case WobVertexElement::WVTT_BYTEARGB: offset += sizeof(uint32_t); break;
		case WobVertexElement::WVTT_SHORT2:   offset += sizeof(short)*2; break;
		default:
			assert( false && "Inknown vertex type" );
		}
		++type;
	}

	return offset;

}

} // end namespace Graphics