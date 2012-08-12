//!-----------------------------------------------------
//!
//! \file scrconsole.cpp
//! the screen console bits shared between platforms
//!
//!-----------------------------------------------------

#include "dx11.h"
#include "scrconsole.h"
#include <stdarg.h>
#if defined( USE_DIRECTWRITE )
#include <d3d10_1.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

#pragma comment( lib, "d3d10_1" )
#pragma comment( lib, "d2d1" )
#pragma comment( lib, "dwrite" )

extern HWND g_hWnd;

namespace
{
	static const unsigned int s_size = 15;
	static const char s_fontName[] = "Courier New";
	static const unsigned int s_vertspace = 0;
	const static char s_letterTable[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!£$%^&*():;,.<>/?\"\\";
}
#else

#include "texture.h"

#endif

namespace Dx11
{

void ScrConsole::ScrollUp()
{
	for(unsigned int i=1;i < m_sConsole.size();i++)
	{
		m_sConsole[i-1] = m_sConsole[i];
	}
}

void ScrConsole::ScrollDown()
{
	for(unsigned int i=0;i < m_sConsole.size()-1;i++)
	{
		m_sConsole[i+1] = m_sConsole[i];
	}
	m_sConsole[0].m_Text.clear();
}

void ScrConsole::SetCursor(unsigned int line)
{
	while( line >= m_sConsole.size() )
	{
		ScrollUp();
		line--;
	}

	m_iCurLine = line;
}

void ScrConsole::Clear()
{
	for(unsigned int i=0;i < m_sConsole.size();i++)
	{
		m_sConsole[i].m_Text.clear();
	}
	m_iCurLine = 0;
}

void ScrConsole::Print( const std::string& text, TEXT_FLAGS flags, TEXT_COLOURS colour )
{
	m_sConsole[m_iCurLine].m_Text = text;
	m_sConsole[m_iCurLine].m_Flags = flags;
	m_sConsole[m_iCurLine].m_Colour = colour;
	SetCursor( m_iCurLine+1 );
}

void ScrConsole::PrintF( const char *pStr, ... )
{
	char pBuffer[1024];
	va_list marker;
	va_start( marker, pStr );
	vsprintf( pBuffer, pStr, marker );
	Print( pBuffer );
	va_end( marker );
}

#if defined( USE_DIRECTWRITE )

ScrConsole::ScrConsole() :
//	m_pFont(0),
	m_iCurLine(0) {
	HRESULT hr;

	// Direct3D 10.1 Device and Swapchain creation
	hr = D3D10CreateDevice1(
					Gfx::Get()->GetDxgiAdapter(),
					D3D10_DRIVER_TYPE_HARDWARE,
					0,
					D3D10_CREATE_DEVICE_BGRA_SUPPORT ,
					D3D10_FEATURE_LEVEL_10_0,
					D3D10_1_SDK_VERSION,
					&m_pDevice
					);


    // Create Direct2D factory.
    hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &m_pD2DFactory
        );

    // Create a shared DirectWrite factory.
    if (SUCCEEDED(hr)) {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
            );
    }

    // Create a text format using Gabriola with a font size of 72.
    // This sets the default font, weight, stretch, style, and locale.
    if (SUCCEEDED(hr))
    {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Gabriola",                // Font family name.
            NULL,                       // Font collection (NULL sets it to use the system font collection).
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            72.0f,
            L"en-us",
            &m_pTextFormat
            );
    }

    //get the dpi information
//    HDC screen = GetDC(0);
//  dpiScaleX = GetDeviceCaps(screen, LOGPIXELSX) / 96.0f;
//    dpiScaleY = GetDeviceCaps(screen, LOGPIXELSY) / 96.0f;
//    ReleaseDC(0, screen);
	FLOAT dpiX;
	FLOAT dpiY;
	m_pD2DFactory->GetDesktopDpi(&dpiX, &dpiY);

	// Create the DXGI Surface Render Target.

	D2D1_RENDER_TARGET_PROPERTIES props =
		D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
			dpiX,
			dpiY
			);

	m_bufferHandle = TextureHandle::Create( &Texture::CreationStruct( 
			Texture::CF_RENDER_TARGET | Texture::CF_KEYEDMUTEX, 
			Graphics::Gfx::Get()->GetScreenWidth(), Graphics::Gfx::Get()->GetScreenHeight(), 
			DXGI_FORMAT_B8G8R8A8_UNORM | Texture::TF_PLATFORM_SPECIFIC, 1, 0 ) );
	m_bufferTex = m_bufferHandle->Acquire();

	// obtain handle to IDXGIResource object.
	
	// QI IDXGIResource interface to synchronized shared surface.
	IDXGIResource* pDXGIResource = NULL;
	DXFAIL( m_bufferTex->m_baseTexture->QueryInterface(__uuidof(IDXGIResource), (LPVOID*) &pDXGIResource) );
	DXFAIL( pDXGIResource->GetSharedHandle(&m_hSharedHandle) );
	// QI IDXGIKeyedMutex interface of synchronized shared surface's resource handle.
	DXFAIL( pDXGIResource->QueryInterface( __uuidof(IDXGIKeyedMutex), (LPVOID*)&m_keyedMutex ) );
	pDXGIResource->Release();

	// Obtain handle to Sync Shared Surface created by Direct3D11 Device.
	IDXGISurface1* sharedSurf;
	DXFAIL( m_pDevice->OpenSharedResource( m_hSharedHandle,__uuidof(IDXGISurface1), (LPVOID*) &sharedSurf) );

	DXFAIL( m_pD2DFactory->CreateDxgiSurfaceRenderTarget( sharedSurf,  &props, &m_pBackBufferRT ) );
	sharedSurf->Release();

	DXFAIL( m_pBackBufferRT->CreateSolidColorBrush( D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush ) );


    //// Center align (horizontally) the text.
    //if (SUCCEEDED(hr))
    //{
    //    hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    //}

    //if (SUCCEEDED(hr))
    //{
    //    hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    //}

	m_sConsole.resize( Gfx::Get()->GetScreenHeight() / (s_size + s_vertspace) );

//	DXFAIL( m_pFont->PreloadText( s_letterTable, (INT) strlen(s_letterTable) ) );


	m_BlendShaderHandle = ShaderFXHandle::Load( "FullScreenPorterDuff" );

	ShaderFXPtr effect = m_BlendShaderHandle->Acquire();
	effect->setGlobalVariable( "Tex", m_bufferHandle );

}

ScrConsole::~ScrConsole(){
	m_BlendShaderHandle->Close();
	m_bufferTex.reset();
	m_bufferHandle->Close();
	SAFE_RELEASE( m_keyedMutex );
 	SAFE_RELEASE( m_pBlackBrush );
	SAFE_RELEASE( m_pBackBufferRT );
	SAFE_RELEASE( m_pTextFormat );
	SAFE_RELEASE( m_pDWriteFactory );
	SAFE_RELEASE( m_pD2DFactory );
}

void ScrConsole::Display() {

	DWORD result = m_keyedMutex->AcquireSync( 0, 5 );
	if( result != WAIT_OBJECT_0 ) {
		// skip this frame TODO better sharing!!
		Log << "m_keyedMutex skip warning\n";
		m_keyedMutex->ReleaseSync( 0 );
		return;
	}

	m_pBackBufferRT->BeginDraw();
	m_pBackBufferRT->SetTransform(D2D1::Matrix3x2F::Identity());

	m_pBackBufferRT->Clear( D2D1::ColorF(D2D1::ColorF::AliceBlue) );
	HRESULT hr;
	int i=0;
	std::vector<LineData>::const_iterator ldIt = m_sConsole.begin();
	while( ldIt != m_sConsole.end() ) {
		if( !ldIt->m_Text.empty() ) {
			// TODO blinking and alpha
			DWORD colour = (ldIt->m_Colour & ~0xFF000000) | 0xFF000000;

			unsigned int format = DT_NOCLIP;
			if( ldIt->m_Flags & SCTL_LEFT )
				format |= DT_LEFT;
			if( ldIt->m_Flags & SCTL_CENTER )
				format |= DT_CENTER;
			if( ldIt->m_Flags & SCTL_RIGHT )
				format |= DT_RIGHT;

			D2D1_RECT_F rect;
			rect.left = 0; rect.right = Gfx::Get()->GetScreenWidth();
			rect.top = i * (s_size +s_vertspace);
			rect.bottom = rect.top + (s_size +s_vertspace);

			
			WCHAR tmp[ 2048 ];
			int cchUTF16 = ::MultiByteToWideChar(
				CP_UTF8,                // convert from UTF-8
				MB_ERR_INVALID_CHARS,   // error on invalid chars
				ldIt->m_Text.c_str(),   // source UTF-8 string
				ldIt->m_Text.size()+1,	// total length of source UTF-8 string in CHAR's (= bytes), including end-of-string \0
				tmp,                   
				2048                       
				);

			// Use the DrawText method of the D2D render target interface to draw.
			m_pBackBufferRT->DrawText(
				tmp,        // The string to render.
				cchUTF16,    // The string's length.
				m_pTextFormat,    // The text format.
				rect,       // The region of the window where the text will be rendered.
				m_pBlackBrush     // The brush used to draw the text.
				);
		}
							
		++ldIt;
		++i;
	}
	m_pBackBufferRT->EndDraw();

	// now finished with the direct2d 10.1 device, so release
	// give it to the d3d11 device and blend it onto the backbuffer
	m_keyedMutex->ReleaseSync( 1 );


	m_keyedMutex->AcquireSync( 1, 5 );
	if( result != WAIT_OBJECT_0 ) {
		// skip this frame TODO better sharing!!
		Log << "m_keyedMutex skip warning\n";
		m_keyedMutex->ReleaseSync( 1 );
		return;
	}

	ShaderFXPtr effect = m_BlendShaderHandle->Acquire();

	D3DX11_TECHNIQUE_DESC techDesc;
	effect->m_pEffectTechnique->GetDesc( &techDesc );

	Gfx::Get()->GetMainContext()->IASetVertexBuffers( 0, 0, NULL, NULL, NULL );
	Gfx::Get()->GetMainContext()->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	Gfx::Get()->GetMainContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

	for( uint32_t i = 0;i < techDesc.Passes;++i) {
		effect->m_pEffectTechnique->GetPassByIndex( i )->Apply( 0,  Gfx::Get()->GetMainContext() );
		Gfx::Get()->GetMainContext()->Draw( 1, 0 );
	}
	m_keyedMutex->ReleaseSync( 0 );
}

#else

ScrConsole::ScrConsole() :
	m_iCurLine(0) {

	m_pFontTexture = TextureHandle::Load( "Font" );

	m_sConsole.resize( Gfx::Get()->getScreenHeight() / 10 );

}

ScrConsole::~ScrConsole(){
	m_pFontTexture->Close(); 
}

void ScrConsole::Display() {
	using namespace Graphics;
	float fBBWidth = ( float ) Gfx::Get()->getScreenWidth();
	float fBBHeight = ( float ) Gfx::Get()->getScreenHeight();;

    float fCharTexSizeX = 0.010526315f;
    float fGlyphSizeX = 24.0f / fBBWidth;
    float fGlyphSizeY = 50.0f / fBBHeight;

	float fRectLeft = -1.0f;
	float fRectTop = -1.0f;
	float fOriginalLeft = fRectLeft;

	int lineNum=0;
	std::vector<LineData>::const_iterator ldIt = m_sConsole.begin();
	while( ldIt != m_sConsole.end() ) {
		if( !ldIt->m_Text.empty() ) {
			// TODO blinking and alpha
			DWORD colour = (ldIt->m_Colour & ~0xFF000000) | 0xFF000000;

			const char* strText = ldIt->m_Text.c_str();
			int numChars = ldIt->m_Text.size();

			// SCTL_LFET is default so do nothing
			if( ldIt->m_Flags & SCTL_LEFT ) {
			}
			if( ldIt->m_Flags & SCTL_CENTER ) {
				fRectLeft = 0.0f - ((numChars * fCharTexSizeX) / 2.0f);
			}
			if( ldIt->m_Flags & SCTL_RIGHT ) {
				fRectLeft = 1.0f - ((numChars+numChars/2) * fCharTexSizeX) - fCharTexSizeX/2;
			}

			for( int i=0; i< numChars; i++ ) {
				if( strText[i] < 32 || strText[i] > 126 ) {
					continue;
				}
				float uvl = (strText[i] - 32) * fCharTexSizeX;

				Core::RGBAColour rgbaColour = Core::RGBAColour::unpackARGB( colour );
				rgbaColour *= 1.0f - ((lineNum) * (1.0f/50.f));

				// draw glypth
				SpriteEngine::Get()->texturedRect( m_pFontTexture, ImageComposer::NORMAL, 
																Math::Vector2(fRectLeft+fGlyphSizeX/2, fRectTop+fGlyphSizeY/2), 
																Math::Vector2(fGlyphSizeX/2,fGlyphSizeY/2),
																rgbaColour,
																Math::Vector2(uvl + (1.0f/950.f), 0), 
																Math::Vector2(uvl + fCharTexSizeX + (1.0f/950.f), 1), 
																0 );

				fRectLeft += fGlyphSizeX;
			}

			fRectLeft = fOriginalLeft;
			fRectTop += fGlyphSizeY/2;
			lineNum++;
		}
		++ldIt;
	}
}
#endif

}
