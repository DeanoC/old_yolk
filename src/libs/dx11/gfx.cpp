//!-----------------------------------------------------
//!
//! \file graphics_pc.cpp
//! the main graphics singleton
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "core/resourceman.h"
#include "resourceloader.h"
#include "programman.h"
#include "texture.h"
#include "screen.h"		
#include "rendercontext.h"
#include "renderstates.h"
#include "scene/pipeline.h"
#include "scene/imagecomposer.h"
#include "scene/debugprims.h"
#include "core/platform_windows/win_shell.h"
#include <boost/lexical_cast.hpp>
#include "gfx.h"

namespace Dx11
{

Gfx::Gfx() {
	using namespace Scene;

	HRESULT hr;
	IDXGIFactory1* factory;
	// enumerate all adaptors 
	DXFAIL( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**) &factory ) );
	do {
		IDXGIAdapter1* adapter;
		hr = factory->EnumAdapters1( (UINT)adapters.size(), &adapter );
		if( hr == S_OK ) {
			adapters.push_back( DXGIAdapterPtr(adapter,false) );
		}
	} while( hr == S_OK );
	factory->Release();

	UINT dx11CreateDeviceFlags = 0;
#if defined( _DEBUG )
	dx11CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	// TODO proper device enumeration
	// For now first device on first adaptor
	adapter = adapters[0];
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Device*			dvc;
	ID3D11DeviceContext*	ctx;
	DXGIAdapterPtr adapt;
	DXFAIL( D3D11CreateDevice( 	adapter.get(), D3D_DRIVER_TYPE_UNKNOWN, NULL, 
								dx11CreateDeviceFlags, 
								featureLevels, NUM_ARRAY_ELEMENTS(featureLevels), 
								D3D11_SDK_VERSION, &dvc, &featureLevel, &ctx ) );
	device = D3DDevicePtr( dvc, false );

	ResourceLoader::derivedInit( CORE_NEW ResourceLoader(this) );
	renderContexts.push_back( std::shared_ptr<Scene::RenderContext>( 
												CORE_NEW RenderContext( ctx ) ) );

	ProgramMan::derivedInit( CORE_NEW ProgramMan() );
	ProgramMan::getr().initDefaultPrograms();

	// create some stock samplers
	SamplerState::CreationInfo pcsci = {
		FM_MIN_MAG_MIP_POINT, AM_CLAMP, AM_CLAMP, AM_CLAMP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX
	};
	SamplerStateHandle::create( SAMPLER_STATE_POINT_CLAMP, &pcsci, Core::RMRF_DONTFLUSH );
	SamplerState::CreationInfo lcsci = {
		FM_MIN_MAG_MIP_LINEAR, AM_CLAMP, AM_CLAMP, AM_CLAMP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX
	};
	SamplerStateHandle::create( SAMPLER_STATE_LINEAR_CLAMP, &lcsci, Core::RMRF_DONTFLUSH );
	SamplerState::CreationInfo acsci = {
		FM_ANISOTROPIC, AM_CLAMP, AM_CLAMP, AM_CLAMP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX, 16
	};
	SamplerStateHandle::create( SAMPLER_STATE_ANISO16_CLAMP, &acsci, Core::RMRF_DONTFLUSH );
	SamplerState::CreationInfo wpcsci = {
		FM_MIN_MAG_MIP_POINT, AM_WRAP, AM_WRAP, AM_WRAP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX
	};
	SamplerStateHandle::create( SAMPLER_STATE_POINT_WRAP, &wpcsci, Core::RMRF_DONTFLUSH );
	SamplerState::CreationInfo wlcsci = {
		FM_MIN_MAG_MIP_LINEAR, AM_WRAP, AM_WRAP, AM_WRAP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX
	};
	SamplerStateHandle::create( SAMPLER_STATE_LINEAR_WRAP, &wlcsci, Core::RMRF_DONTFLUSH );
	SamplerState::CreationInfo wacsci = {
		FM_ANISOTROPIC, AM_WRAP, AM_WRAP, AM_WRAP, CF_ALWAYS, 0.0f, -FLT_MAX, FLT_MAX, 16
	};
	SamplerStateHandle::create( SAMPLER_STATE_ANISO16_WRAP, &wacsci, Core::RMRF_DONTFLUSH );

	RasteriserState::CreationInfo nrsci = {
		RasteriserState::CreationInfo::MULTISAMPLE, FIM_FILL, CUM_BACK, 0, 0, 0
	};
	RasteriserStateHandle::create( RENDER_STATE_NORMAL, &nrsci, Core::RMRF_DONTFLUSH );
	RasteriserState::CreationInfo nncrsci = {
		RasteriserState::CreationInfo::MULTISAMPLE, FIM_FILL, CUM_NONE, 0, 0, 0
	};
	RasteriserStateHandle::create( RENDER_STATE_NORMAL_NOCULL, &nncrsci, Core::RMRF_DONTFLUSH );

	DepthStencilState::CreationInfo ndsci = {
		(DepthStencilState::CreationInfo::FLAGS)(DepthStencilState::CreationInfo::DEPTH_ENABLE | DepthStencilState::CreationInfo::DEPTH_WRITE), 
		CF_LESS
	};
	DepthStencilStateHandle::create( DEPTH_STENCIL_STATE_NORMAL, &ndsci, Core::RMRF_DONTFLUSH );

	DepthStencilState::CreationInfo dlwdsci = {
		(DepthStencilState::CreationInfo::FLAGS)(DepthStencilState::CreationInfo::DEPTH_ENABLE ), 
		CF_LESS
	};
	DepthStencilStateHandle::create( DEPTH_STENCIL_STATE_LESS_NOWRITE, &dlwdsci, Core::RMRF_DONTFLUSH );

	RenderTargetStates::CreationInfo nwnrtci = { (RenderTargetStates::CreationInfo::FLAGS)0, 1, { (TargetState::FLAGS)0, TWE_NONE } };
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_NOWRITE, &nwnrtci, Core::RMRF_DONTFLUSH );
	RenderTargetStates::CreationInfo nrtci = { (RenderTargetStates::CreationInfo::FLAGS)0, 1, { (TargetState::FLAGS)0, TWE_ALL } };
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_NOBLEND_WRITEALL, &nrtci, Core::RMRF_DONTFLUSH );
	RenderTargetStates::CreationInfo nwcrtci = { (RenderTargetStates::CreationInfo::FLAGS)0, 1, { (TargetState::FLAGS)0, TWE_COLOUR } };
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_NOBLEND_WRITECOLOUR, &nwcrtci, Core::RMRF_DONTFLUSH );
	RenderTargetStates::CreationInfo nwartci = { (RenderTargetStates::CreationInfo::FLAGS)0, 1, { (TargetState::FLAGS)0, TWE_ALPHA } };
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_NOBLEND_WRITEALPHA, &nwartci, Core::RMRF_DONTFLUSH );

	RenderTargetStates::CreationInfo artci = {
		(RenderTargetStates::CreationInfo::FLAGS)0, 
		1,	{ TargetState::FLAGS::BLEND_ENABLE, TWE_ALL, 
				BM_SRC_ALPHA, BM_INV_SRC_ALPHA, BO_ADD,		// colour blend
				BM_SRC_ALPHA, BM_INV_SRC_ALPHA, BO_ADD,		// alpha blend
			}
	};
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_OVER_WRITEALL, &artci, Core::RMRF_DONTFLUSH );
	RenderTargetStates::CreationInfo pmartci = {
		(RenderTargetStates::CreationInfo::FLAGS)0, 
		1,	{ TargetState::FLAGS::BLEND_ENABLE, TWE_ALL, 
				BM_ONE, BM_INV_SRC_ALPHA, BO_ADD,		// colour blend
				BM_ONE, BM_INV_SRC_ALPHA, BO_ADD,		// alpha blend
			}
	};
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_PMOVER_WRITEALL, &pmartci, Core::RMRF_DONTFLUSH );

	RenderTargetStates::CreationInfo aartci = {
		(RenderTargetStates::CreationInfo::FLAGS)0, 
		1,	{ TargetState::FLAGS::BLEND_ENABLE, TWE_ALL, 
				BM_ONE, BM_ONE, BO_ADD,		// colour blend
				BM_ONE, BM_ONE, BO_ADD,		// alpha blend
			}
	};
	RenderTargetStatesHandle::create( RENDER_TARGET_STATES_ADD_WRITEALL, &aartci, Core::RMRF_DONTFLUSH );

}

Gfx::~Gfx() {
	hashPipeline.clear();
	pipelines.clear();
	renderContexts.clear();
	screens.clear();
	ProgramMan::shutdown();
	ResourceLoader::shutdown();
	device.reset();
	adapter.reset();
	adapters.clear();
}

Scene::ScreenPtr Gfx::createScreen( uint32_t width, uint32_t height, uint32_t flags ) {
	using namespace Scene;
	HRESULT hr;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
 	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	if( flags & Scene::SCRF_FULLSCREEN ) {
		swapChainDesc.Windowed = false;
	} else {
		swapChainDesc.Windowed = true;
	}
	IDXGIFactory1* 				factory;
	IDXGISwapChain*				swapChain;

	adapter->GetParent( __uuidof( IDXGIFactory1 ), (VOID**)(&factory) );
	DXFAIL( factory->CreateSwapChain( device.get(), &swapChainDesc, &swapChain ) );
	factory->Release();
	ID3D11Texture2D* backBuffer;
	hr = swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( void** )&backBuffer );
	auto cs = Scene::Texture::TextureCtor( 
		RCF_TEX_2D | RCF_OUT_RENDER_TARGET | RCF_D3D_FROM_OS, GTF_UNKNOWN, 0
	);
	cs.referenceTex = backBuffer;
	const std::string backName = "_BackBuffer" + boost::lexical_cast<std::string>( screens.size() );
	TextureHandlePtr backHandle = TextureHandle::create( backName.c_str(), &cs );

	auto screen = std::make_shared<Dx11::Screen>();
	screen->width = width;
	screen->height = height;
	screen->flags = flags;
	screen->swapChain = swapChain;
	screen->renderer = this;
	screen->backHandle.reset( backHandle );
	screen->pointClampSamplerHandle.reset( 	SamplerStateHandle::create( "_SS_Point_Clamp" ) );
	screen->renderTargetWriteHandle.reset( RenderTargetStatesHandle::create( "_RTS_NoBlend_WriteAll" ) );
	screen->copyProgramHandle.reset( ProgramHandle::load( "rendertarget_copy" ) );


	if( flags & SCRF_OVERLAY ) {
		screen->imageComposer.reset( CORE_NEW ImageComposer() );
	}
	if( flags & SCRF_DEBUGPRIMS ) {
		screen->debugPrims.reset( CORE_NEW DebugPrims() );
	}
	// todo stereo

	auto scnscr = std::static_pointer_cast<Scene::Screen>( screen );

	screens.push_back( scnscr );
	return scnscr;
}

void Gfx::destroyScreen( Scene::ScreenPtr screen ) {
	screens.erase( std::find( screens.cbegin(), screens.cend(), screen ) );
}

void Gfx::houseKeep() {
}

concurrency::accelerator_view Gfx::getAMPAcceleratorView() {
	return concurrency::direct3d::create_accelerator_view( device.get() );
}

void Screen::display( Scene::TextureHandlePtr toDisplay ) {
	Scene::RenderContext* ctx = getRenderer()->getPrimaryContext();

	auto backBuffer = backHandle.acquire();

	ctx->bindRenderTarget( backBuffer );

	auto sampler = pointClampSamplerHandle.acquire();
	auto tex = toDisplay->acquire();
	auto prg = copyProgramHandle.acquire();
	auto rtw = renderTargetWriteHandle.acquire();

	Scene::Viewport viewport = {
		0.0f, 0.0f, (float) width, (float) height, 0.0f, 1.0f
	};

	if( tex->getSamples() <= 1 ) {
		// copy
		ctx->bind( viewport );
		ctx->bind( Scene::ST_FRAGMENT, 0, sampler );
		ctx->bind( Scene::ST_FRAGMENT, 0, tex );
		ctx->bind( prg );
		ctx->bind( rtw );
		ctx->draw( Scene::PT_POINT_LIST, 1 );
	} else {
		// TODO Resolve
	}

	if( hasDebugPrims() ) {
		getDebugPrims()->render( ctx );
	}

	ResourceLoader::getr().renderThreadUpdate( getComposer() );

	if( hasOverlay() ) {
		getComposer()->render( ctx );
	}


	ctx->unbindRenderTargets();
	swapChain->Present( 0, 0 );
}

}
