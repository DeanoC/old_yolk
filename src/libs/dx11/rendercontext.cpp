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

	ID3D11RenderTargetView* rtViews[] = { 
		static_cast<ID3D11RenderTargetView*> ( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() ) 
	};
	ctx->OMSetRenderTargets( 1, rtViews, static_cast<ID3D11DepthStencilView*>( depthTarget->getDx11View( Scene::Resource::DEPTH_STENCIL_VIEW )->view.get() ) );
}

void RenderContext::bindRenderTarget( const Scene::TexturePtr& starget ) {
	const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( starget );

	ID3D11RenderTargetView* rtViews[] = { 
		static_cast<ID3D11RenderTargetView*>( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() ) 
	};
	ctx->OMSetRenderTargets( 1, rtViews, nullptr );
}

void RenderContext::bindDepthOnlyRenderTarget( const Scene::TexturePtr& sdepthTarget ) {
	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthTarget );
	ctx->OMSetRenderTargets( 0, nullptr, static_cast<ID3D11DepthStencilView*>( depthTarget->getDx11View( Scene::Resource::DEPTH_STENCIL_VIEW )->view.get() ) );
}


void RenderContext::bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const stargets, const Scene::TexturePtr& sdepthTarget ) {
	ID3D11RenderTargetView* rtViews[8];
	assert( numTargets <= 8 );
	for( unsigned int i=0;i < numTargets;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		rtViews[i] = static_cast<ID3D11RenderTargetView*>( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() );
	};
	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthTarget );
	ID3D11DepthStencilView* dTarget = static_cast<ID3D11DepthStencilView*>( depthTarget->getDx11View( Scene::Resource::DEPTH_STENCIL_VIEW )->view.get() );
	ctx->OMSetRenderTargets( numTargets, rtViews, dTarget );
}
void RenderContext::bindRenderTargets( unsigned int numTargets, const Scene::TexturePtr* const stargets ) {
	ID3D11RenderTargetView* rtViews[8];
	assert( numTargets <= 8 );
	for( unsigned int i=0;i < numTargets;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		rtViews[i] = static_cast<ID3D11RenderTargetView*>( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() );
	};
	ctx->OMSetRenderTargets( numTargets, rtViews, nullptr );
}

void RenderContext::bindUnorderedViews( unsigned int numViews, const Scene::TexturePtr* const stargets ) {
	ID3D11UnorderedAccessView* uavViews[8];
	assert( numViews <= 8 );
	for( unsigned int i=0;i < numViews;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		uavViews[i] = static_cast<ID3D11UnorderedAccessView*>( target->getDx11View( Scene::Resource::UNORDERED_ACCESS_VIEW )->view.get() );
	};
	ctx->CSSetUnorderedAccessViews( 0, numViews, uavViews, nullptr );
}

void RenderContext::bindRenderTargetsAndUnorderedViews( unsigned int numTargets, const Scene::TexturePtr* const stargets, const Scene::TexturePtr& sdepthtarget, unsigned int numViews, const Scene::ViewPtr* const sviews ) {
	ID3D11RenderTargetView* rtViews[8];
	ID3D11UnorderedAccessView* uavViews[8];
	assert( numViews + numTargets <= 8 );
	assert( numTargets <= 8 );

	const std::shared_ptr<Dx11::Texture> depthTarget = std::static_pointer_cast<Dx11::Texture>( sdepthtarget );
	ID3D11DepthStencilView* dTarget = static_cast<ID3D11DepthStencilView*>( depthTarget->getDx11View( Scene::Resource::DEPTH_STENCIL_VIEW )->view.get() );

	for( unsigned int i=0;i < numTargets;++i ){ 
		const std::shared_ptr<Dx11::Texture> target = std::static_pointer_cast<Dx11::Texture>( stargets[i] );
		rtViews[i] = static_cast<ID3D11RenderTargetView*>( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() );
	};

	assert( numViews <= 8 );
	for( unsigned int i=0;i < numViews;++i ){ 
		const std::shared_ptr<Dx11View> target = std::static_pointer_cast<Dx11View>( sviews[i] );
		uavViews[i] = static_cast<ID3D11UnorderedAccessView*>( target->view.get() );
	};

	ctx->OMSetRenderTargetsAndUnorderedAccessViews( numTargets, rtViews, dTarget, numTargets, numViews, uavViews, nullptr );

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
					auto buf = static_cast<ID3D11Buffer*>( cb->get().get() );
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
	auto view = static_cast<ID3D11ShaderResourceView* const>( tex->getDx11View( tex->SHADER_RESOURCE_VIEW )->view.get() );
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
	auto view = static_cast<ID3D11ShaderResourceView* const>( buf->getDx11View( buf->SHADER_RESOURCE_VIEW )->view.get() );
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
	auto samp = static_cast<ID3D11SamplerState*>( sampler->samplerState.get() );
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
	auto states = static_cast<ID3D11BlendState* const>( targetStates->renderTargetState.get() );
	// TODO blend factor and sample mask
	float todo[4] = { 1,1,1,1 };
	ctx->OMSetBlendState( states, todo, 0xFFFFFFFF );
}

void RenderContext::bind( const Scene::DepthStencilStatePtr& _sdsState, uint32_t _stencilRef ) {
	auto dsState = std::static_pointer_cast<Dx11::DepthStencilState>( _sdsState );
	auto state = static_cast<ID3D11DepthStencilState* const>( dsState->depthStencilState.get() );
	// todo stencil ref
	ctx->OMSetDepthStencilState( state, _stencilRef );
}

void RenderContext::bind( const Scene::RasteriserStatePtr& sraster ) {
	auto raster = std::static_pointer_cast<Dx11::RasteriserState>( sraster );
	auto state = static_cast<ID3D11RasterizerState* const>( raster->rasteriserState.get() );
	ctx->RSSetState( state );
}

void RenderContext::bind( const Scene::VertexInputPtr& svertexInput ) {
	auto vin = std::static_pointer_cast<Dx11::VertexInput>( svertexInput );

IF_DEBUG_START
	if( !vin->inputLayout ) {
		LOG(INFO) << "VertexInput must be first validated with a gpu program that accepts this input\n";
	}
IF_DEBUG_END
	CORE_ASSERT( vin->inputLayout );

	std::shared_ptr<DataBuffer> buffers[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
	ID3D11Buffer* d3dBuffers[ Scene::VertexInput::MAX_ELEMENT_COUNT ];
	UINT offsets[ Scene::VertexInput::MAX_ELEMENT_COUNT ] ;
	int bufferCount = 0;
	for( int i = 0; i < vin->streamCount; ++i ) {
		// bind vertex buffers
		if( vin->streamBuffers[i] ) {
			buffers[ bufferCount ] = std::static_pointer_cast<Dx11::DataBuffer>( vin->streamBuffers[i]->acquire() );
			d3dBuffers[ bufferCount ] = static_cast<ID3D11Buffer*>(buffers[ bufferCount ]->get().get() );
			bufferCount++;
		} else {
			d3dBuffers[ i ] = nullptr;
		}
		offsets[ i ] = 0;
	}

	// an vertex input can just be the layout, with the buffers set letter (faster for same layout across many vbs)
	if( bufferCount > 0 ) {
		ctx->IASetVertexBuffers( 0 , bufferCount, d3dBuffers, vin->streamStrides, offsets );
	}
	ctx->IASetInputLayout( vin->inputLayout.get() );
}

void RenderContext::bindCB( const Scene::SHADER_TYPES type, const uint32_t unit, const Scene::DataBufferPtr& stex ) {
	auto buf = std::static_pointer_cast<Dx11::DataBuffer>( stex );
	auto buff = static_cast<ID3D11Buffer* const>( buf->get().get() );
	switch( type ) {
	case Scene::ST_VERTEX: ctx->VSSetConstantBuffers( unit,1, &buff ); break;
	case Scene::ST_FRAGMENT: ctx->PSSetConstantBuffers( unit,1, &buff ); break;
	case Scene::ST_GEOMETRY: ctx->GSSetConstantBuffers( unit,1, &buff ); break;
	case Scene::ST_HULL: ctx->HSSetConstantBuffers( unit,1, &buff ); break;
	case Scene::ST_DOMAIN: ctx->DSSetConstantBuffers( unit,1, &buff ); break;
	case Scene::ST_COMPUTE: ctx->CSSetConstantBuffers( unit,1, &buff ); break;
	default:;
	}
}

void RenderContext::bindVB( const unsigned int _stream, const Scene::DataBufferPtr& svb, const unsigned int _stride, const unsigned int _offset ) {
	auto vb = std::static_pointer_cast<Dx11::DataBuffer>( svb );
	auto buf = static_cast<ID3D11Buffer* const>( vb->get().get() );
	ctx->IASetVertexBuffers( _stream , 1, &buf, &_stride, &_offset );

}

void RenderContext::bindIB( const Scene::DataBufferPtr& sib, const unsigned int _stride ) {
	auto ib = std::static_pointer_cast<Dx11::DataBuffer>( sib );
	auto buf = static_cast<ID3D11Buffer* const>( ib->get().get() );
	ctx->IASetIndexBuffer( buf, (_stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0 );
}

void RenderContext::clear( const Scene::TexturePtr& starget, const Core::Colour& colour ) {
	const std::shared_ptr<Dx11::Texture> target = 
				std::static_pointer_cast<Dx11::Texture>( starget );
	ctx->ClearRenderTargetView( static_cast<ID3D11RenderTargetView*>( target->getDx11View( Scene::Resource::RENDER_TARGET_VIEW )->view.get() ),
								(float*) &colour.getRGBAColour()[0] );
}

void RenderContext::clear( const Scene::TexturePtr& starget, bool clearDepth, float depth, bool clearStencil, uint8_t stencil ) {
	const std::shared_ptr<Dx11::Texture> target = 
				std::static_pointer_cast<Dx11::Texture>( starget );
	ctx->ClearDepthStencilView( static_cast<ID3D11DepthStencilView*>(target->getDx11View( Scene::Resource::DEPTH_STENCIL_VIEW )->view.get() ),
								(clearDepth ? D3D11_CLEAR_DEPTH : 0) | (clearStencil ? D3D11_CLEAR_STENCIL : 0),
								depth, stencil );
}

void RenderContext::clear( const Scene::ViewPtr& starget ) {
	static const uint32_t zeros[] = { 0,0,0,0 };
	const std::shared_ptr<Dx11::Dx11View> target = 
				std::static_pointer_cast<Dx11::Dx11View>( starget );
	ctx->ClearUnorderedAccessViewUint( static_cast<ID3D11UnorderedAccessView*>( target->view.get() ), zeros );
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
void RenderContext::copy(	const Scene::TexturePtr& sdst, const int dstX, const int dstY, const int dstZ, 
							const Scene::TexturePtr& ssrc, const int srcX, const int srcY, const int srcZ, const int srcWidth, const int srcHeight, const int srcDepth ) {
	auto dt = std::static_pointer_cast<Dx11::Texture>( sdst );
	auto st = std::static_pointer_cast<Dx11::Texture>( ssrc );

	D3D11_BOX box;
	box.left = srcX;
	box.top = srcY;
	box.front = srcZ;
	box.right = srcX + srcWidth;
	box.bottom = srcY + srcHeight;
	box.back = srcZ + srcDepth;

	ctx->CopySubresourceRegion( dt->get().get(), 0, dstX, dstY, dstZ, st->get().get(), 0, &box );

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