//!-----------------------------------------------------
//!
//! \file scrconsole_pc.h
//! contains the PC bits of an onscreen output console
//!
//!-----------------------------------------------------

private:
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