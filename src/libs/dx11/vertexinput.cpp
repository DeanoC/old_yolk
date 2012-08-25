//!-----------------------------------------------------
//!
//! \file texture_pc.cpp
//! the texture functions on PC
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "gfx.h"
#include "rendercontext.h"
#include "program.h"
#include "vertexinput.h"

namespace Dx11 {

void VertexInput::validate( Scene::ProgramPtr sprg ) {
	using namespace Scene;
	if( isValid() == false ) {
		auto prg = std::static_pointer_cast<Program>( sprg );

		HRESULT hr;
		ID3D11InputLayout* ip;
		SHADER_TYPES stype = ST_VERTEX;
		if( !prg->shader[ stype ] ) {
			stype = ST_GEOMETRY;
		}
		hr = Gfx::getr()()->CreateInputLayout( 	vertexElements, elementCount, 
												prg->src[stype]->GetBufferPointer(), prg->src[stype]->GetBufferSize(), &ip );
		inputLayout = D3DInputLayoutPtr( ip, false );
		valid = true;
	}
}

Scene::VertexInput* VertexInput::internalCreate( const Scene::VertexInput::CreationInfo* creation ) {
	using namespace Scene;

	CORE_ASSERT( creation->elementCount < Scene::VertexInput::MAX_ELEMENT_COUNT );

	VertexInput* vi = CORE_NEW VertexInput;
	vi->elementCount = creation->elementCount;
	vi->streamCount = 0;

	size_t streamOffset[ MAX_ELEMENT_COUNT + 1] = { 0 };
	memset( vi->streamStrides, 0, sizeof(size_t) * MAX_ELEMENT_COUNT );
	memset( vi->streamBuffers, 0, sizeof(DataBufferHandlePtr) * MAX_ELEMENT_COUNT );

	uint32_t offset = 0;
	// once first to calc auto strides and offsets also D3D names etc.
	for( int i=0; i < creation->elementCount; ++i ) {
		const VinElement* ourtype = &creation->elements[i];
		D3D11_INPUT_ELEMENT_DESC* d3dtype = &vi->vertexElements[i];

		uint32_t beginOffset = offset;
		switch( ourtype->usage ) {
		case VE_POSITION:	d3dtype->SemanticName = "POSITION";		d3dtype->SemanticIndex = 0; break;
		case VE_NORMAL:		d3dtype->SemanticName = "NORMAL"; 		d3dtype->SemanticIndex = 0; break;
		case VE_BINORMAL:	d3dtype->SemanticName = "BINORMAL";		d3dtype->SemanticIndex = 0; break;
		case VE_TANGENT:	d3dtype->SemanticName = "TANGENT"; 		d3dtype->SemanticIndex = 0; break;
		case VE_TEXCOORD0:	d3dtype->SemanticName = "TEXCOORD"; 	d3dtype->SemanticIndex = 0; break;
		case VE_TEXCOORD1:	d3dtype->SemanticName = "TEXCOORD"; 	d3dtype->SemanticIndex = 1; break;
		case VE_TEXCOORD2:	d3dtype->SemanticName = "TEXCOORD"; 	d3dtype->SemanticIndex = 2; break;
		case VE_TEXCOORD3:	d3dtype->SemanticName = "TEXCOORD"; 	d3dtype->SemanticIndex = 3; break;
		case VE_COLOUR0:	d3dtype->SemanticName = "COLOR"; 		d3dtype->SemanticIndex = 0; break;
		case VE_COLOUR1: 	d3dtype->SemanticName = "COLOR"; 		d3dtype->SemanticIndex = 1; break;
		default:			CORE_ASSERT( false && "Unknown vertex usage" );
		}
		switch( ourtype->type ) {
		case VT_FLOAT4:		d3dtype->Format = DXGI_FORMAT_R32G32B32A32_FLOAT;break;
		case VT_FLOAT3:		d3dtype->Format = DXGI_FORMAT_R32G32B32_FLOAT;break;
		case VT_FLOAT2:		d3dtype->Format = DXGI_FORMAT_R32G32_FLOAT;break;
		case VT_FLOAT1:		d3dtype->Format = DXGI_FORMAT_R32_FLOAT;break;
		case VT_BYTEARGB:	d3dtype->Format = DXGI_FORMAT_B8G8R8A8_UNORM;break;
		case VT_SHORT2:		d3dtype->Format = DXGI_FORMAT_R16G16_UINT;break;
		default:			CORE_ASSERT( false && "Inknown vertex type" );
		}

		CORE_ASSERT( creation->data[i].buffer != NULL );
		if( !vi->streamBuffers[creation->data[i].stream] ) {
			vi->streamBuffers[creation->data[i].stream ] = creation->data[i].buffer;
		}
		vi->streamCount = Math::Max( vi->streamCount, (int) creation->data[i].stream + 1 );
		if( creation->data[i].stride == VI_AUTO_STRIDE ) {
			vi->streamStrides[ creation->data[i].stream ] += getElementSize( creation->elements[i] );
		} else {
			// if multiple elements to the same stride set different strides, its undefined
			vi->streamStrides[ creation->data[i].stream ] = creation->data[i].stride;
		}

		size_t offset = 0;
		if( creation->data[i].offset == VI_AUTO_OFFSET ) {
			offset = streamOffset[ i ];
		} else {
			offset = creation->data[i].offset;
		}
		streamOffset[ i + 1 ] = offset + getElementSize( creation->elements[i] );

		d3dtype->InputSlot = creation->data[i].stream;
		d3dtype->AlignedByteOffset = (UINT) offset;
		d3dtype->InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		d3dtype->InstanceDataStepRate = 0;
	}

	return vi;
}

}