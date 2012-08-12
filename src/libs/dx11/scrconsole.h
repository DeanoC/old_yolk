//!-----------------------------------------------------
//!
//! \file scrconsole.h
//! contains an onscreen output console
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_SCRCONSOLE_H_)
#define YOLK_DX11_SCRCONSOLE_H_
#pragma once

//forward decl
struct IDX10Device1;
struct ID2D1Factory;
struct ID2D1SolidColorBrush;
struct ID2D1RenderTarget;
struct IDWriteFactory;
struct IDWriteTextFormat;
struct ID2D1HwndRenderTarget;
#include "texture.h"

#if !defined( YOLK_DX11_SHADER_H_ )
#include "shader.h"
#endif

#include <vector>

namespace Dx11
{
	class ScrConsole : public Core::Singleton<ScrConsole> {
	public:
		enum TEXT_FLAGS
		{
			SCTL_LEFT			= 0x1,
			SCTL_CENTER			= 0x2,
			SCTL_RIGHT			= 0x4,
		};
		enum TEXT_COLOURS
		{
			SCTC_BLACK		= 0x00000000,
			SCTC_WHITE		= 0x00FFFFFF,
			SCTC_RED		= 0x00FF0000,
			SCTC_GREEN		= 0x0000FF00,
			SCTC_BLUE		= 0x000000FF,
			SCTC_BLINKING	= 0x80000000,
		};
		
		ScrConsole();
		~ScrConsole();

		void Print( const std::string& text, TEXT_FLAGS flags = SCTL_LEFT, TEXT_COLOURS colour = SCTC_GREEN );

		void PrintF( const char* pStr, ... );

		void SetCursor( unsigned int line );

		void ScrollUp();
		void ScrollDown();
		void Clear();

		void Display();

	private:
		struct LineData {
			std::string		m_Text;
			TEXT_FLAGS		m_Flags;
			TEXT_COLOURS	m_Colour;
		};

		unsigned int m_iCurLine;
		std::vector<LineData>		m_sConsole;

#if defined( USE_DIRECTWRITE )
	    // Direct2D
		ID3D10Device1*			m_pDevice;
	    ID2D1Factory*			m_pD2DFactory;
	    ID2D1SolidColorBrush*	m_pBlackBrush;
	    ID2D1RenderTarget*		m_pBackBufferRT;
		HANDLE					m_hSharedHandle;
		IDXGIKeyedMutex*		m_keyedMutex;
		TextureHandlePtr		m_bufferHandle;
		TexturePtr				m_bufferTex;
		ShaderFXHandlePtr		m_BlendShaderHandle;

	    // DirectWrite
	    IDWriteFactory*		m_pDWriteFactory;
	    IDWriteTextFormat*	m_pTextFormat;
#else
		TextureHandlePtr	m_pFontTexture;
#endif

	};
}

#endif //WIERD_GRAPHICS_SCRCONSOLE_H
