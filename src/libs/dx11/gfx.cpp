//!-----------------------------------------------------
//!
//! \file graphics_pc.cpp
//! the main graphics singleton
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "vbmanager.h"
#include "ibmanager.h"
#include "core/resourceman.h"
#include "wobfile.h"
#include "vdeclmanager.h"
#include "shadermanager.h"
#include "rendermatrixcache.h"
#include "gfx_debug_render.h"
#include "graphics.h"
#include "texture.h"
#include "spriteengine.h"
#include "rendercontext.h"
#include <Strsafe.h>
#include "core/platform_windows/win_shell.h"

//--------------------------------------------------------------------------------------
// Direct3D9 dynamic linking support -- calls top-level D3D9 APIs with graceful
// failure if APIs are not present.
//--------------------------------------------------------------------------------------

// Function prototypes
#include "d3d9.h"
typedef INT         (WINAPI * LPD3DPERF_BEGINEVENT)(D3DCOLOR, LPCWSTR);
typedef INT         (WINAPI * LPD3DPERF_ENDEVENT)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETMARKER)(D3DCOLOR, LPCWSTR);
typedef VOID        (WINAPI * LPD3DPERF_SETREGION)(D3DCOLOR, LPCWSTR);
typedef BOOL        (WINAPI * LPD3DPERF_QUERYREPEATFRAME)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETOPTIONS)( DWORD dwOptions );
typedef DWORD       (WINAPI * LPD3DPERF_GETSTATUS)( void );
typedef HRESULT     (WINAPI * LPCREATEDXGIFACTORY)(REFIID, void ** );
typedef HRESULT     (WINAPI * LPD3D11CREATEDEVICE)( IDXGIAdapter*, D3D_DRIVER_TYPE, HMODULE, UINT32, D3D_FEATURE_LEVEL*, UINT, UINT32, ID3D11Device**, D3D_FEATURE_LEVEL*, ID3D11DeviceContext** );

// Module and function pointers
static HMODULE                              s_hModD3D9 = NULL;
static LPD3DPERF_BEGINEVENT                 s_DynamicD3DPERF_BeginEvent = NULL;
static LPD3DPERF_ENDEVENT                   s_DynamicD3DPERF_EndEvent = NULL;
static LPD3DPERF_SETMARKER                  s_DynamicD3DPERF_SetMarker = NULL;
static LPD3DPERF_SETREGION                  s_DynamicD3DPERF_SetRegion = NULL;
static LPD3DPERF_QUERYREPEATFRAME           s_DynamicD3DPERF_QueryRepeatFrame = NULL;
static LPD3DPERF_SETOPTIONS                 s_DynamicD3DPERF_SetOptions = NULL;
static LPD3DPERF_GETSTATUS                  s_DynamicD3DPERF_GetStatus = NULL;
static HMODULE                              s_hModDXGI = NULL;
static LPCREATEDXGIFACTORY                  s_DynamicCreateDXGIFactory = NULL;
static HMODULE                              s_hModD3D11 = NULL;
static LPD3D11CREATEDEVICE                  s_DynamicD3D11CreateDevice = NULL;

// Ensure function pointers are initialized
static bool DXUT_EnsureD3D9APIs( void )
{
	// If the module is non-NULL, this function has already been called.  Note
	// that this doesn't guarantee that all ProcAddresses were found.
	if( s_hModD3D9 != NULL )
		return true;

	// This may fail if Direct3D 9 isn't installed
	s_hModD3D9 = LoadLibrary( "d3d9.dll" );
	if( s_hModD3D9 != NULL )
	{
		s_DynamicD3DPERF_BeginEvent = (LPD3DPERF_BEGINEVENT)GetProcAddress( s_hModD3D9, "D3DPERF_BeginEvent" );
		s_DynamicD3DPERF_EndEvent = (LPD3DPERF_ENDEVENT)GetProcAddress( s_hModD3D9, "D3DPERF_EndEvent" );
		s_DynamicD3DPERF_SetMarker = (LPD3DPERF_SETMARKER)GetProcAddress( s_hModD3D9, "D3DPERF_SetMarker" );
		s_DynamicD3DPERF_SetRegion = (LPD3DPERF_SETREGION)GetProcAddress( s_hModD3D9, "D3DPERF_SetRegion" );
		s_DynamicD3DPERF_QueryRepeatFrame = (LPD3DPERF_QUERYREPEATFRAME)GetProcAddress( s_hModD3D9, "D3DPERF_QueryRepeatFrame" );
		s_DynamicD3DPERF_SetOptions = (LPD3DPERF_SETOPTIONS)GetProcAddress( s_hModD3D9, "D3DPERF_SetOptions" );
		s_DynamicD3DPERF_GetStatus = (LPD3DPERF_GETSTATUS)GetProcAddress( s_hModD3D9, "D3DPERF_GetStatus" );
	}

	return s_hModD3D9 != NULL;
}

//#define NSIGHT_FIX
//#define USE_REF

namespace Graphics
{

Gfx::SHADER_MODEL	Gfx::s_eMaxShaderModel = Gfx::SM5_0;

Gfx::Gfx() :
	m_device(0),
	m_iScreenWidth(0),
	m_iScreenHeight(0),
	m_eShaderModel( Gfx::SM4_0 ) {

	DXUT_EnsureD3D9APIs();

#if defined( NSIGHT_FIX )
	HRESULT hr;
	// enumerate all adaptors and outputs
	DXFAIL( CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), (void**) &factory ) );

	do {
		IDXGIAdapter1* adapter;
		hr = factory->EnumAdapters1( adapters.size(), &adapter );
		if( hr == S_OK ) {
			adapters.push_back( adapter );
		}
	} while( hr == S_OK );
#else
	adapters.push_back( NULL );
#endif

	m_frameCount = 0;
}

Gfx::~Gfx() {
}

bool Gfx::createScreen(unsigned int iWidth, unsigned int iHeight, bool bFullScreen, ANTI_ALIASING aaSetting ) {
	HRESULT hr;

	UINT dx11CreateDeviceFlags = 0;

#if defined( _DEBUG )
	dx11CreateDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#if defined( USE_REF )
	dx11CreateDeviceFlags |= D3D11_CREATE_DEVICE_SWITCH_TO_REF;
#endif
#endif

	static const D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

#if defined( NSIGHT_FIX )
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_UNKNOWN;
#else
	const D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
#endif

	// TODO proper device enumeration
	// For now first HW device on first adaptor
	DXGIAdapterPtr adapt;
	DXFAIL( D3D11CreateDevice( adapt, driverType, NULL, dx11CreateDeviceFlags, featureLevels, NUM_ARRAY_ELEMENTS(featureLevels), D3D11_SDK_VERSION, &m_device, &m_featureLevel, &m_mainContext ) );
	adapters.push_back( adapt );
	switch( m_featureLevel ) {
		case D3D_FEATURE_LEVEL_11_0: m_eShaderModel = SM5_0; break;
		case D3D_FEATURE_LEVEL_10_1: m_eShaderModel = SM4_1; break;
		case D3D_FEATURE_LEVEL_10_0: m_eShaderModel = SM4_0; break;
	}
	m_eShaderModel = std::min( m_eShaderModel, s_eMaxShaderModel );

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = g_hWnd;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.BufferDesc.Width = iWidth;
	swapChainDesc.BufferDesc.Height = iHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
 
	// we don't use swap chain MSAA, we do a manual resolve to here when using MSAA
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	if( bFullScreen == false ) {
		swapChainDesc.Windowed = true;
	} else {
		swapChainDesc.Windowed = false;
	}

#if !defined( NSIGHT_FIX )
	IDXGIDevice1* dxgiDev = 0;
	m_device->QueryInterface( IID_IDXGIDevice1, (VOID**)(&dxgiDev) );
	dxgiDev->GetParent( IID_IDXGIAdapter1, (VOID**)(&adapters.front()) );
	if( adapters.front() != NULL ) {
		adapters.front()->GetParent( IID_IDXGIFactory1, (VOID**)(&factory) );
	} else {
		dxgiDev->GetParent( IID_IDXGIAdapter, (VOID**)(&adapters.front()) );
		assert( adapters.front() );
		adapters.front()->GetParent( IID_IDXGIFactory, (VOID**)(&factory) );
	}
	dxgiDev->Release();
#endif

	DXFAIL( factory->CreateSwapChain( m_device, &swapChainDesc, &m_swapChain ) );

	// for now
	m_adapter = adapters.front();

#if !defined( NSIGHT_FIX )
#if defined( _DEBUG )
	// this report live objects now, which isn't useful but left here to cut and paste if wanted
	ID3D11Debug* pDebug = 0;
	m_device->QueryInterface( IID_ID3D11Debug, (VOID**)(&pDebug) );
	if( pDebug ) {
//		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
//		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY);
		pDebug->Release();
	}
#endif
#endif
#if defined( USE_REF )
	// this is reported not to work on Direct3D 11 I kid you not!
	ID3D11SwitchToRef* pRef = 0;
	m_device->QueryInterface( IID_ID3D11SwitchToRef, (VOID**)(&pRef) );
	if( pRef ) {
		pRef->SetUseRef(TRUE);
		pRef->Release();
	}
#endif
#if defined( _DEBUG )
	// this report live objects now, which isn't useful but left here to cut and paste if wanted
	ID3D11InfoQueue* pInfoQueue = 0;
	m_device->QueryInterface( IID_ID3D11InfoQueue, (VOID**)(&pInfoQueue) );
	if( pInfoQueue ) {
		pInfoQueue->SetMuteDebugOutput( FALSE );
		pInfoQueue->Release();
	}
#endif

	m_iScreenWidth = iWidth;
	m_iScreenHeight = iHeight;
	m_aaSetting = aaSetting;

	DXFAIL( D3DX11CreateThreadPump( 0, 0, &m_threadPump ) );
	
	// required before any shaders etc are loaded
	InstallResourceTypes();

	IDXGISurface *pDxgiSurface = NULL;
	hr = m_swapChain->GetBuffer( 0, __uuidof( *pDxgiSurface ), ( LPVOID* )&pDxgiSurface );
	m_pDxgiBackBufferSurface = pDxgiSurface;

	ID3D11Texture2D* pBackBuffer;
	hr = m_swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );

	TextureHandlePtr backHandle = TextureHandle::Create( "_BackBuffer", &Texture::CreationStruct(	
														Texture::CF_FROM_D3D_TEXTURE | Texture::CF_RENDER_TARGET, 
														0, 0, TF_UNKNOWN, 0, (uintptr_t) pBackBuffer ) );
	m_backBufferTex = backHandle->Acquire();

	backHandle->Close();

	VBManager::Init();
	IBManager::Init();
	VDeclManager::Init();

	ShaderManager::Init();
	// let shader manager set defines for multi-sampling, to allow compile time changes required
	// by the shaders
	D3D_SHADER_MACRO msShaderMacro;
	msShaderMacro.Name = "MULTISAMPLE_SAMPLES";
	switch ( m_aaSetting )
	{
	default:
	case AA_NONE:
		msShaderMacro.Definition = "0"; 
		break;
	case AA_2X:
		msShaderMacro.Definition = "2"; 
		break;
	case AA_4X:
		msShaderMacro.Definition = "4"; 
		break;
	case AA_8X:
		msShaderMacro.Definition = "8"; 
		break;
	}
	ShaderManager::Get()->d3dXShaderMacros.insert( ShaderManager::Get()->d3dXShaderMacros.begin(), msShaderMacro );

	m_mainRenderContext = CORE_NEW RenderContext;
	m_mainRenderContext->matrixCache = CORE_NEW RenderMatrixCache();
	m_mainRenderContext->viewFrustum = NULL;
	m_mainRenderContext->deviceContext = m_mainContext;

	// the debug renderer requires shaders so after InstallResourceTypes()
	GfxDebugRender::Init();

	// the sprite engine requires shaders
	SpriteEngine::Init();

	m_defaultStateHandle = ShaderFXHandle::Load( "DefaultState" );
	m_defaultTextureHandle = TextureHandle::Load( "Defaulted" );
	m_solidWireShaderHandle = ShaderFXHandle::Load( "SolidWire" );

	for( int i = 0 ;i < Gfx::MAX_GLOBAL_TEXTURES;++i ) {
		m_globalTextures[i] = getDefaultedTexture();
	}

	// temp memory for random data
	{
		uint8_t* mem = CORE_NEW uint8_t[(m_backBufferTex->m_iWidth/2) * (m_backBufferTex->m_iHeight/2)];
		for( unsigned int i=0;i < (m_backBufferTex->m_iWidth/2) * (m_backBufferTex->m_iHeight/2);++i ) {
			mem[i] = (uint8_t)floorf( ((float)rand() / (float)RAND_MAX) * 256.f);
		}

		// create the screen space (2x2 quad) random offset, this allows per pixel (well quad) to just look
		// up a random number. This is constant per frame and is just used to break up patterns
		m_globalTextures[ GT_SCREEN_RANDOM ] = TextureHandle::Create( "_ScreenRandom", &Texture::CreationStruct( 
							Texture::CF_PRE_FILL, 
							m_backBufferTex->m_iWidth/2, m_backBufferTex->m_iHeight/2, 
							DXGI_FORMAT_R8_SINT, 1, 0, (uintptr_t) mem, (m_backBufferTex->m_iWidth/2)* sizeof(uint8_t) ) );

		CORE_DELETE mem;
	}

	// normal encode and decode texture (cubemap and a 2D)
	// mip 0 = 512x512x6 = 1,572,864 normals stored in 21 bits
	// mip 1 = 256x256x6 = 393,216 normals stored in 19 bits
	// mip 2 = 128x128x6 = 98,304 normals stored in 17 bits
	// total space = 2,064,384 * 4 bytes = 8,257,536 bytes
	{
		const uint32_t totalSizeInBytes = ((512*512) + (256*256) + (128*128)) * 6 * 4;
		uint32_t* mem = (uint32_t*) CORE_STACK_ALLOC( totalSizeInBytes );
		uint32_t* baseAddr = mem;
		uint32_t index = 0;
		for( int face = 0; face < 6; ++face ) {
			uint32_t* mip0 = baseAddr;
			uint32_t* mip1 = mip0 + (512*512);
			uint32_t* mip2 = mip1 + (256*256); 
			for( unsigned int i=0;i < 512;++i ) {
				for( unsigned int j=0;j < 512;++j ) {
					mip0[ (i*512) + j ] = index;
					// these and and shift ensure indices align up and down mips
					// doesn't really matter as not likely to be used, but it makes me 
					// feel better to know you can step up and down the mip chain (with care!)
					// and its all aligns
					if( (i & 0x1) == 0x0 && (j & 0x1) == 0x0 ) {
						mip1[ ((i>>1)*256) + (j>>1) ] = index>>1;
						if( (i & 0x2) == 0x2 && (j & 0x2) == 0x2 ) {
							mip2[ ((i>>2)*128) + (j>>2) ] = index>>2;
						}
					}
					index++;
				}
			}
			baseAddr = baseAddr + ((512*512) + (256*256) + (128*128));
		}
		m_globalTextures[ GT_NORMAL_ENCODE ] = TextureHandle::Create( "_NormalEncode", 
			&Texture::CreationStruct( 
				Texture::CF_PRE_FILL | Texture::CF_CUBE_MAP,
				512, 512, 
				DXGI_FORMAT_R32_UINT, 3, 0, (uintptr_t) mem, 512 * sizeof(uint32_t) ) );

		/* TODO decode
		Math::Vector3 cubeFace[6] = {
			Math::Vector3( +1, 0, 0 ), // face 0 = +x
			Math::Vector3( -1, 0, 0 ), // face 1 = -x
			Math::Vector3( 0, +1, 0 ), // face 2 = +y
			Math::Vector3( 0, -1, 0 ), // face 3 = -y
			Math::Vector3( 0, 0, +1 ), // face 4 = +z
			Math::Vector3( 0, 0, -1 ), // face 5 = -z
		};
		uint32_t* baseAddr = mem;
		uint32_t index = 0;
		for( int face = 0; face < 6; ++face ) {
			uint32_t* mip0 = baseAddr;
			uint32_t* mip1 = mip0 + (512*512);
			uint32_t* mip2 = mip1 + (256*256); 
			for( unsigned int i=0;i < 512;++i ) {
				for( unsigned int j=0;j < 512;++j ) {
					mip0[ (i*512) + j ] = index;
					// these and and shift ensure indices align up and down mips
					// doesn't really matter as not likely to be used, but it makes me 
					// feel better to know you can step up and down the mip chain (with care!)
					// and its all aligns
					if( (i & 0x1) == 0x0 && (j & 0x1) == 0x0 ) {
						mip1[ ((i>>1)*256) + (j>>1) ] = index>>1;
						if( (i & 0x2) == 0x2 && (j & 0x2) == 0x2 ) {
							mip2[ ((i>>2)*128) + (j>>2) ] = index>>2;
						}
					}
					index++;
				}
			}
			baseAddr = baseAddr + ((512*512) + (256*256) + (128*128));
		}*/

	}


	return true;
}

void Gfx::shutdownScreen() {
	m_globalTextures[ GT_NORMAL_ENCODE ]->Close();
	m_globalTextures[ GT_SCREEN_RANDOM ]->Close();

	m_solidWireShaderHandle->Close();
	m_defaultStateHandle->Close();
	m_defaultTextureHandle->Close();

	m_mainContext->ClearState();
	m_mainContext->Flush();
	SpriteEngine::Shutdown();
	GfxDebugRender::Shutdown();
	ShaderManager::Shutdown();
	VDeclManager::Shutdown();
	IBManager::Shutdown();
	VBManager::Shutdown();

	SAFE_RELEASE( m_pDxgiBackBufferSurface );
	if( m_threadPump ) {
		m_threadPump->PurgeAllItems();
	}
	SAFE_RELEASE( m_threadPump );
	SAFE_RELEASE( m_mainContext );
	SAFE_RELEASE( m_device );
	SAFE_RELEASE( m_swapChain );

	CORE_DELETE m_mainRenderContext->matrixCache;
	CORE_DELETE m_mainRenderContext;
}

void Gfx::present() {
	assert( m_swapChain );
	m_swapChain->Present( 0, 0 );
}

void Gfx::beginFrame( bool bClear ) {

//	HRESULT hr;
//	DXFAIL( D3DX11UnsetAllDeviceObjects( m_mainContext ) );

	++m_frameCount;
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (float) getScreenWidth();
	vp.Height = (float) getScreenHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_mainContext->RSSetViewports( 1, &vp );

	resetToDefaultRenderTargets();

	if( bClear ) {
		// Just clear the backbuffer
		float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //red,green,blue,alpha
		m_mainContext->ClearRenderTargetView( (ID3D11RenderTargetView*)m_backBufferTex->m_extraView, ClearColor );
	}
	ShaderFXPtr defaultState = m_defaultStateHandle->Acquire();
	defaultState->m_pEffectTechnique->GetPassByIndex( 0 )->Apply( 0,  Gfx::Get()->GetMainContext() );

	static void* nulls[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT] = {0};
	m_mainContext->PSSetShaderResources( 0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, (ID3D11ShaderResourceView**)nulls );
	m_mainContext->PSSetSamplers( 0, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT, (ID3D11SamplerState**)nulls );

	m_totalPrimitiveCount = 0;
	m_passPrimitiveCount = 0;

}

void Gfx::endFrame() {
	HRESULT hr;
	DXFAIL( m_threadPump->ProcessDeviceWorkItems( 50 ) );

#if defined( _DEBUG )
	// this report live objects now, which isn't useful but left here to cut and paste if wanted
	ID3D11InfoQueue* pInfoQueue = 0;
	m_device->QueryInterface( IID_ID3D11InfoQueue, (VOID**)(&pInfoQueue) );
	if( pInfoQueue ) {
		for( UINT64 i = 0; i < pInfoQueue->GetNumStoredMessages(); ++i ) {
			// Get the size of the message
			{
				SIZE_T messageLength = 0;
				HRESULT hr = pInfoQueue->GetMessage(0, NULL, &messageLength);
				// Allocate space and get the message
				D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)CORE_STACK_ALLOC(messageLength);
				hr = pInfoQueue->GetMessage(0, pMessage, &messageLength);
				LOG(INFO) << pMessage->pDescription << "\n";
			}
		}
		pInfoQueue->ClearStoredMessages();
		pInfoQueue->Release();
	}
#endif


	SpriteEngine::Get()->Render();
	GfxDebugRender::Get()->Flush();
}

void Gfx::resetToDefaultRenderTargets() {

	ID3D11RenderTargetView* rtViews[] = { (ID3D11RenderTargetView*)m_backBufferTex->m_extraView };
	m_mainContext->OMSetRenderTargets( 1, rtViews, 0 );
}

void Gfx::pushDebugMarker( const char* text ) {
	WCHAR tmp[ 2048 ];
	int cchUTF16 = ::MultiByteToWideChar(
		CP_UTF8,                // convert from UTF-8
		MB_ERR_INVALID_CHARS,   // error on invalid chars
		text,					// source UTF-8 string
		strlen(text)+1,			// total length of source UTF-8 string in CHAR's (= bytes), including end-of-string \0
		tmp,                   
		2048                       
		);
	s_DynamicD3DPERF_BeginEvent( D3DCOLOR_XRGB( rand() % 255, rand() % 255, rand() % 255 ), tmp );
}

void Gfx::popDebugMarker() {
	s_DynamicD3DPERF_EndEvent();
}

}