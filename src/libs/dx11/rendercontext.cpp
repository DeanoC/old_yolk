//!-----------------------------------------------------
//!
//! \file rendercontext.cpp
//! Dx11 implementation of the render context interface
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "databuffer.h"
#include "texture.h"
#include "program.h"
#include "vertexinput.h"
#include "renderstates.h"
#include "core/resourceman.h"
#include "rendercontext.h"

namespace Dx11 {

RenderContext::RenderContext( D3DDeviceContextPtr dvc ) :
	ctx(dvc) {
}

void RenderContext::pushDebugMarker( const char* text ) const {
}

void RenderContext::popDebugMarker() const {
}

void RenderContext::bindRenderTargets( const Scene::TexturePtr& starget, 
										const Scene::TexturePtr& sdepthTarget ) {
	const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( starget );
	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthTarget );

	ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)target->getView( Resource::RENDER_TARGET_VIEW ).get() };
	ctx->OMSetRenderTargets( 1, rtViews, (ID3D11DepthStencilView*)depthTarget->getView( Resource::DEPTH_STENCIL_VIEW ).get() );
}

void RenderContext::bindRenderTarget( const Scene::TexturePtr& starget ) {
	const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( starget );

	ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)target->getView( Resource::RENDER_TARGET_VIEW ).get()};
	ctx->OMSetRenderTargets( 1, rtViews, nullptr );
}

void RenderContext::bindDepthOnlyRenderTarget( const Scene::TexturePtr& sdepthTarget ) {
	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthTarget );
	ctx->OMSetRenderTargets( 0, nullptr, (ID3D11DepthStencilView*)depthTarget->getView( Resource::DEPTH_STENCIL_VIEW ).get() );
}


void RenderContext::bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const stargets, const Scene::TexturePtr& sdepthTarget ) {
	ID3D11RenderTargetView* rtViews[8];
	assert( numTargets < 8 );
	for( unsigned int i=0;i < numTargets;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		rtViews[i] = (ID3D11RenderTargetView*)target->getView( Resource::RENDER_TARGET_VIEW ).get();
	};
	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthTarget );
	ID3D11DepthStencilView* dTarget = (ID3D11DepthStencilView*) depthTarget->getView( Resource::DEPTH_STENCIL_VIEW ).get();
	ctx->OMSetRenderTargets( numTargets, rtViews, dTarget );
}
void RenderContext::bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const stargets ) {
	ID3D11RenderTargetView* rtViews[8];
	assert( numTargets < 8 );
	for( unsigned int i=0;i < numTargets;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		rtViews[i] = (ID3D11RenderTargetView*)target->getView( Resource::RENDER_TARGET_VIEW ).get();
	};
	ctx->OMSetRenderTargets( numTargets, rtViews, nullptr );
}

void RenderContext::bindUnorderedViews( unsigned int numViews, const Scene::TexturePtr* const stargets ) {
	ID3D11UnorderedAccessView* uavViews[8];
	assert( numViews < 8 );
	for( unsigned int i=0;i < numViews;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		uavViews[i] = (ID3D11UnorderedAccessView*)target->getView( Resource::UNORDERED_ACCESS_VIEW ).get();
	};
	ctx->CSSetUnorderedAccessViews( 0, numViews, uavViews, nullptr );
}


void RenderContext::bind( const Scene::Viewport& viewport ) {
	ctx->RSSetViewports( 1, (D3D11_VIEWPORT*) &viewport );
}

void RenderContext::bind( const Scene::ProgramPtr& sprg ) {
	auto prg = std::static_pointer_cast<Dx11::Program>( sprg );
	// TODO optimise this rather than a double loop and function calls and lots of evil!
	for( int i = 0; i < Scene::MAX_SHADER_TYPES;++i ) {	
		if( prg->shader[i] ) {
			for( int j = 0; j < Scene::CF_NUM_BLOCKS; ++j ) {
				if( prg->usedBuffers & BIT(j) ) {
					auto cb = std::static_pointer_cast<Dx11::DataBuffer>( constantCache.getBlock((Scene::CONSTANT_FREQ_BLOCKS)j)->acquire() );
					auto buf = (ID3D11Buffer*) cb->get().get();
					// TODO cache which are set
					switch( i ) {
					case Scene::ST_VERTEX: ctx->VSSetConstantBuffers( j, 1, &buf ); break;
					case Scene::ST_FRAGMENT: ctx->PSSetConstantBuffers( j, 1, &buf ); break;
					case Scene::ST_GEOMETRY: ctx->GSSetConstantBuffers( j, 1, &buf ); break;
					case Scene::ST_HULL: ctx->HSSetConstantBuffers( j, 1, &buf ); break;
					case Scene::ST_DOMAIN: ctx->DSSetConstantBuffers( j, 1, &buf ); break;
					case Scene::ST_COMPUTE: ctx->CSSetConstantBuffers( j, 1, &buf ); break;
					default:;
					}
				}
			}
			switch( i ) {
				case Scene::ST_VERTEX: {
					auto shader = (ID3D11VertexShader*) prg->shader[i].get();
					ctx->VSSetShader( shader, nullptr, 0 ); 
				} break;

				case Scene::ST_FRAGMENT: {
					auto shader = (ID3D11PixelShader*) prg->shader[i].get();
					ctx->PSSetShader( shader, nullptr, 0 ); 
				} break;
				case Scene::ST_GEOMETRY: {
					auto shader = (ID3D11GeometryShader*) prg->shader[i].get();
					ctx->GSSetShader( shader, nullptr, 0 ); 
				} break;
				case Scene::ST_HULL: {
					auto shader = (ID3D11HullShader*) prg->shader[i].get();
					ctx->HSSetShader( shader, nullptr, 0 ); 
				} break;
				case Scene::ST_DOMAIN: {
					auto shader = (ID3D11DomainShader*) prg->shader[i].get();
					ctx->DSSetShader( shader, nullptr, 0 ); 
				} break;
				case Scene::ST_COMPUTE: {
					auto shader = (ID3D11ComputeShader*) prg->shader[i].get();
					ctx->CSSetShader( shader, nullptr, 0 );
					threadGroupXSize = prg->threadGroupXSize;
					threadGroupYSize = prg->threadGroupYSize;
					threadGroupZSize = prg->threadGroupZSize;
				} break;
				default:;
			}
		} else {
			switch( i ) {
				case Scene::ST_VERTEX: {
					ctx->VSSetShader( nullptr, nullptr, 0 ); 
				} break;
				case Scene::ST_FRAGMENT: {
					ctx->PSSetShader( nullptr, nullptr, 0 ); 
				} break;
				case Scene::ST_GEOMETRY: {
					ctx->GSSetShader( nullptr, nullptr, 0 ); 
				} break;
				case Scene::ST_HULL: {
					ctx->HSSetShader( nullptr, nullptr, 0 ); 
				} break;
				case Scene::ST_DOMAIN: {
					ctx->DSSetShader( nullptr, nullptr, 0 ); 
				} break;
				case Scene::ST_COMPUTE: {
					ctx->CSSetShader( nullptr, nullptr, 0 ); 
				} break;
				default:;
			}
		}
	}
}

void RenderContext::bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::TexturePtr& stex ) {
	auto tex = std::static_pointer_cast<Dx11::Texture>( stex );
	auto view = (ID3D11ShaderResourceView* const) tex->getView( tex->SHADER_RESOURCE_VIEW ).get();
	switch( type ) {
	case Scene::ST_VERTEX: ctx->VSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_FRAGMENT: ctx->PSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_GEOMETRY: ctx->GSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_HULL: ctx->HSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_DOMAIN: ctx->DSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_COMPUTE: ctx->CSSetShaderResources( unit,1, &view ); break;
	default:;
	}
}

void RenderContext::bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::DataBufferPtr& stex ) {
	auto buf = std::static_pointer_cast<Dx11::DataBuffer>( stex );
	auto view = (ID3D11ShaderResourceView* const) buf->getView( buf->SHADER_RESOURCE_VIEW ).get();
	switch( type ) {
	case Scene::ST_VERTEX: ctx->VSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_FRAGMENT: ctx->PSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_GEOMETRY: ctx->GSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_HULL: ctx->HSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_DOMAIN: ctx->DSSetShaderResources( unit,1, &view ); break;
	case Scene::ST_COMPUTE: ctx->CSSetShaderResources( unit,1, &view ); break;
	default:;
	}
}

void RenderContext::bind( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::SamplerStatePtr& ssampler ) {
	auto sampler = std::static_pointer_cast<Dx11::SamplerState>( ssampler );
	auto samp = query_interface<ID3D11SamplerState>( sampler->samplerState, IID_ID3D11SamplerState ).get();
	switch( type ) {
	case Scene::ST_VERTEX: ctx->VSSetSamplers( unit,1, &samp ); break;
	case Scene::ST_FRAGMENT: ctx->PSSetSamplers( unit,1, &samp ); break;
	case Scene::ST_GEOMETRY: ctx->GSSetSamplers( unit,1, &samp ); break;
	case Scene::ST_HULL: ctx->HSSetSamplers( unit,1, &samp ); break;
	case Scene::ST_DOMAIN: ctx->DSSetSamplers( unit,1, &samp ); break;
	case Scene::ST_COMPUTE: ctx->CSSetSamplers( unit,1, &samp ); break;
	default:;
	}
}


void RenderContext::bind( const Scene::RenderTargetStatesPtr& stargetStates ) {
	auto targetStates = std::static_pointer_cast<Dx11::RenderTargetStates>( stargetStates );
	auto states = (ID3D11BlendState* const)targetStates->renderTargetState.get();
	// TODO blend factor and sample mask
	float todo[4] = { 1,1,1,1 };
	ctx->OMSetBlendState( states, todo, 0xFFFFFFFF );
}
void RenderContext::bind( const Scene::DepthStencilStatePtr& sdsState ) {
	auto dsState = std::static_pointer_cast<Dx11::DepthStencilState>( sdsState );
	auto state = (ID3D11DepthStencilState* const)dsState->depthStencilState.get();
	// todo stencil ref
	ctx->OMSetDepthStencilState( state, 0 );
}
void RenderContext::bind( const Scene::RasteriserStatePtr& sraster ) {
	auto raster = std::static_pointer_cast<Dx11::RasteriserState>( sraster );
	auto state = (ID3D11RasterizerState* const)raster->rasteriserState.get();
	ctx->RSSetState( state );
}
void RenderContext::bind( const Scene::VertexInputPtr& svertexInput ) {
	auto vin = std::static_pointer_cast<Dx11::VertexInput>( svertexInput );

	CORE_ASSERT( vin->inputLayout );

	std::shared_ptr<DataBuffer> buffers[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
	ID3D11Buffer* d3dBuffers[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
	UINT offsets[ Scene::VertexInput::MAX_ELEMENT_COUNT ] ;
	for( int i = 0; i < vin->streamCount; ++i ) {
		// bind vertex buffers
		buffers[ i ] = std::static_pointer_cast<Dx11::DataBuffer>( vin->streamBuffers[i]->acquire() );
		d3dBuffers[ i ] = (ID3D11Buffer*)buffers[i]->get().get();
		offsets[ i ] = 0;
	}
	ctx->IASetVertexBuffers( 0 , vin->streamCount, d3dBuffers, vin->streamStrides, offsets );
	ctx->IASetInputLayout( vin->inputLayout.get() );
}

void RenderContext::bindIndexBuffer( const Scene::DataBufferPtr& sib, int indexBytes ) {
	auto ib = std::static_pointer_cast<Dx11::DataBuffer>( sib );
	auto buf = (ID3D11Buffer* const)ib->get().get();
	ctx->IASetIndexBuffer( buf, (indexBytes == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0 );
}

void RenderContext::clear( const Scene::TexturePtr& starget, const Core::Colour& colour ) {
	const std::shared_ptr<Dx11::Texture> target = 
				std::static_pointer_cast<Dx11::Texture>( starget );
	ctx->ClearRenderTargetView( (ID3D11RenderTargetView*)target->getView( Resource::RENDER_TARGET_VIEW ).get(),
								(float*) &colour.getRGBAColour()[0] );
}

void RenderContext::clearDepthStencil( const Scene::TexturePtr& starget, bool clearDepth, float depth, bool clearStencil, uint8_t stencil ) {
	const std::shared_ptr<Dx11::Texture> target = 
				std::static_pointer_cast<Dx11::Texture>( starget );
	ctx->ClearDepthStencilView( (ID3D11DepthStencilView*)target->getView( Resource::DEPTH_STENCIL_VIEW ).get(),
								(clearDepth ? D3D11_CLEAR_DEPTH : 0) | (clearStencil ? D3D11_CLEAR_STENCIL : 0),
								depth, stencil );

}

void RenderContext::copy( const Scene::DataBufferPtr& sdst, const Scene::DataBufferPtr& ssrc ) {
	auto db = std::static_pointer_cast<Dx11::DataBuffer>( sdst );
	auto sb = std::static_pointer_cast<Dx11::DataBuffer>( ssrc );
	ctx->CopyResource( db->get().get(), sb->get().get() );
}
void RenderContext::copy( const Scene::TexturePtr& sdst, const Scene::TexturePtr& ssrc ) {
	auto dt = std::static_pointer_cast<Dx11::Texture>( sdst );
	auto st = std::static_pointer_cast<Dx11::Texture>( ssrc );
	ctx->CopyResource( dt->get().get(), st->get().get() );
}

static D3D11_PRIMITIVE_TOPOLOGY PT_Map[] = {
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,
};
void RenderContext::draw( Scene::PRIMITIVE_TOPOLOGY topo, uint32_t vertexCount, uint32_t startVertex ) {
	ctx->IASetPrimitiveTopology( PT_Map[ topo ] );
	ctx->Draw( vertexCount, startVertex );
}

void RenderContext::drawIndexed( Scene::PRIMITIVE_TOPOLOGY topo, uint32_t indexCount, uint32_t startIndex, uint32_t baseOffset ) {
	ctx->IASetPrimitiveTopology( PT_Map[ topo ] );
	ctx->DrawIndexed( indexCount, startIndex, baseOffset );
}
void RenderContext::dispatch( uint32_t xThreads, uint32_t yThreads, uint32_t zThreads ) {
	ctx->Dispatch( xThreads / threadGroupXSize, yThreads / threadGroupYSize, zThreads / threadGroupZSize );
}

void RenderContext::unbindRenderTargets() {
	ctx->OMSetRenderTargetsAndUnorderedAccessViews( 0, nullptr, nullptr, 0, 0, nullptr, nullptr );
}
void RenderContext::unbindTexture( const Scene::SHADER_TYPES type, const uint32_t unit, const uint32_t count ) {
	static ID3D11ShaderResourceView* const nadaArray[16] = { 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0 };
	switch( type ) {
	case Scene::ST_VERTEX: ctx->VSSetShaderResources( unit, count, nadaArray ); break;
	case Scene::ST_FRAGMENT: ctx->PSSetShaderResources( unit, count, nadaArray ); break;
	case Scene::ST_GEOMETRY: ctx->GSSetShaderResources( unit, count, nadaArray ); break;
	case Scene::ST_HULL: ctx->HSSetShaderResources( unit, count, nadaArray ); break;
	case Scene::ST_DOMAIN: ctx->DSSetShaderResources( unit, count, nadaArray ); break;
	case Scene::ST_COMPUTE: ctx->CSSetShaderResources( unit, count, nadaArray ); break;
	default:;
	}
}
void RenderContext::unbindUnorderedViews() {
	static ID3D11UnorderedAccessView* const nadaArray[8] = { 0, 0, 0, 0,  0, 0, 0, 0 };
	ctx->CSSetUnorderedAccessViews( 0, 8, nadaArray, nullptr );
}
}