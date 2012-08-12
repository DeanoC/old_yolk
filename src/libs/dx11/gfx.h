//!-----------------------------------------------------
//!
//! \file graphics.h
//! Contains the main singleton for the graphics subsystem
//!
//!-----------------------------------------------------
#if !defined(YOLK_DX11_GFX_H_)
#define YOLK_DX11_GFX_H_

#pragma once

#if !defined( YOLK_CORE_LINEAR_ALLOCATOR_H_ )
#include "core/linear_allocator.h"
#endif

#if !defined( YOLK_DX11_TEXTURE_H_ )
#include "dx11/texture.h"
#endif

#if !defined( YOLK_DX11_MATERIAL_H_ )
#include "material.h"
#endif

namespace Dx11 {

	class RenderContext;

	//! The main singleton for the gfx subject
	class Gfx : public Core::Singleton<Gfx> {
	public:
		//! enum passed to create screen to decide what AA we want
		enum ANTI_ALIASING {
			AA_NONE = 0,
			AA_2X	= 2,
			AA_4X	= 4,
			AA_8X	= 8,
		};

		//! enum that gives a idea of the maximum shader version supported by this card
		enum SHADER_MODEL {
			SM4_0,
			SM4_1,
			SM5_0,
		};

		// global system wide textures
		// remember to update the shader binder in shader.cpp if these are changed
		enum GLOBAL_TEXTURES {
			GT_DEPTH_TARGET_TEXTURE,
			GT_OLD_DEPTH_TARGET_TEXTURE,
			GT_OLD_RESULT_TARGET_TEXTURE,
			GT_SCREEN_RANDOM,
			GT_NORMAL_ENCODE,
			GT_NORMAL_DECODE,
			GT_MRT0,
			GT_MRT1,
			GT_MRT2,
			GT_MRT3,
			GT_MRT4,
			GT_MRT5,
			GT_MRT6,
			GT_MRT7,

			MAX_GLOBAL_TEXTURES
		};

		//! dtor
		~Gfx();

		//! Create the visual screen, graphics ops can now comence
		bool createScreen( unsigned int iWidth, unsigned int iHeight, 
							bool bFullScreen, 
							ANTI_ALIASING aaSetting );

		//! close the screen, graphics ops won't work
		void shutdownScreen();

		//! Present the current back buffer to the front
		void present();

		//! marks the beginning of a frame, clears if asked
		void beginFrame( bool bClear );

		//! marks the end of a frame
		void endFrame();

		//! return the defaulted texture
		TextureHandle* getDefaultedTexture() const { return m_defaultTextureHandle; }

		//! returns the width of the current screen/framebuffer
		unsigned int getScreenWidth() { return m_iScreenWidth; }

		//! returns the height of the current screen/framebuffer
		unsigned int getScreenHeight() { return m_iScreenHeight; }

		RenderContext* getMainRenderContext() const { return m_mainRenderContext; }

		int getFrameCount() const { return m_frameCount; }

		ANTI_ALIASING getAntiAliasing() const { return m_aaSetting; }

		TexturePtr getBackBuffer() const { return m_backBufferTex; }

		void pushDebugMarker( const char* text );

		void popDebugMarker();

		const SHADER_MODEL getShaderModel() const { return m_eShaderModel;  }

		static const Core::string& getShaderModelAsString( const SHADER_MODEL sm  ) { 
			static Core::string smStrings[] = {
				"SM4_0",
				"SM4_1",
				"SM5_0",
			};

			return smStrings[ sm ] ; 
		}

		TextureHandlePtr getGlobalTexture( GLOBAL_TEXTURES texIndex ) const { 
															return m_globalTextures[ texIndex ]; }

		void setGlobalTexture( GLOBAL_TEXTURES texIndex, TextureHandlePtr tex) {
															m_globalTextures[ texIndex ] = tex; }

		// returns number of primitives render this pass so far 
		uint32_t getPassPrimitiveCount() const {
			return m_passPrimitiveCount;
		}
		// mark a new pass from primitive count accumulator
		void resetPassPrimitiveCount(){
			m_passPrimitiveCount = 0;
		}

		void incPrimitiveCount( const uint32_t count ) {
			m_totalPrimitiveCount += count;
			m_passPrimitiveCount += count;
		}
	public:
		//! returns the actual D3D Device
		ID3D11Device* GetDevice() const { return m_device; }
		ID3D11DeviceContext* GetMainContext() const { return m_mainContext; }

		ID3DX11ThreadPump* GetThreadPump() const { return m_threadPump; }

		void useRenderTargets( unsigned int numTargets, TexturePtr* pTargets, TexturePtr pDepthTarget );
		void useRenderTarget( TexturePtr pTarget, TexturePtr pDepthTarget );
		void useRenderTarget( TexturePtr pTarget );
		void useDepthOnlyRenderTarget( TexturePtr pDepthTarget );

		void resetToDefaultRenderTargets();

		static void setMaxShaderModel( SHADER_MODEL sm ) { s_eMaxShaderModel = sm; }

		IDXGISurface* GetDxgiBackBuffer() const { return m_pDxgiBackBufferSurface; }

		ShaderFXHandlePtr&		GetSolidWireShaderHandle() { return m_solidWireShaderHandle; }

		IDXGIAdapter* GetDxgiAdapter() const { return m_adapter; }

		typedef Core::list< DXGIAdapterPtr > AdapterContainer;
	private:
		_com_ptr_t< _com_IIID< IDXGIFactory1, &IID_IDXGIFactory1> > factory;
		AdapterContainer		adapters;

		ID3D11Device*			m_device;		//!< the device object
		IDXGISwapChain*			m_swapChain;
		IDXGIAdapter1*			m_adapter;

		ShaderFXHandlePtr		m_defaultStateHandle;
		ShaderFXHandlePtr		m_solidWireShaderHandle;

		D3D_FEATURE_LEVEL		m_featureLevel;
		ID3D11DeviceContext*	m_mainContext;
		IDXGISurface*			m_pDxgiBackBufferSurface;

		ID3DX11ThreadPump*		m_threadPump;

		static SHADER_MODEL	s_eMaxShaderModel;	//!< the maximum shader mode we want to use
	private:
		friend class Core::Singleton<Gfx>;
		//! ctor
		Gfx();

		void InstallResourceTypes();			//!< tell the resource manager what we have

		unsigned int		m_iScreenWidth;		//!< width in pixel of the framebuffer
		unsigned int		m_iScreenHeight;	//!< height in pixels of the framebuffer
		int					m_frameCount;
		ANTI_ALIASING		m_aaSetting;
		TexturePtr			m_backBufferTex;
		SHADER_MODEL		m_eShaderModel;		//!< What is the maximum shader version supported

		TextureHandle*		m_defaultTextureHandle;
		RenderContext*		m_mainRenderContext;
		TextureHandlePtr	m_globalTextures[ MAX_GLOBAL_TEXTURES ];

		uint32_t m_totalPrimitiveCount;
		uint32_t m_passPrimitiveCount;

	};
}


#endif
