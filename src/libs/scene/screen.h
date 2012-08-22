#pragma once
//!-----------------------------------------------------
//!
//! \file screen.h
//! Contains a screen, a screen is a single output
//! device, a renderer can sometimes have a few, 
//! some multi-screens however appear as one screen
//! (eye-finity and dual stereoscopic)
//! conceptually all screens have a framebuffer hence
//! dimensions and samples, this is generally used as the
//! final composited buffer before being displayed in some
//! way.
//!
//!-----------------------------------------------------
#if !defined( YOLK_SCENE_SCREEN_H_ )
#define YOLK_SCENE_SCREEN_H_

#include "texture.h"
#include "debugprims.h"
#include "imagecomposer.h"

namespace Scene {
	class DebugPrims;
	class ImageComposer;
	class Renderer;

	enum SCR_FLAGS {
		SCRF_STEREO			= BIT(0),
		SCRF_OVERLAY		= BIT(1),
		SCRF_FULLSCREEN		= BIT(2),
		SCRF_DEBUGPRIMS		= BIT(3),
	};

	class Screen {
	public:
		virtual ~Screen(){}

		unsigned int getWidth() const { return width; }
		unsigned int getHeight() const { return height; }
		bool isStereo() const { return !!(flags & SCRF_STEREO); }
		bool hasOverlay() const { return !!(flags & SCRF_OVERLAY); }
		bool hasDebugPrims() const { return !!(flags & SCRF_DEBUGPRIMS); }

		virtual void display( TextureHandlePtr toDisplay ) = 0;

		// only valid if the screen has an overlay plane
		ImageComposer* 	getComposer() const { return imageComposer.get(); }
		DebugPrims* 	getDebugPrims() const { return debugPrims.get(); }
		Renderer*		getRenderer() const { return renderer; }
	protected:
		unsigned int width;
		unsigned int height;
		unsigned int flags;

		std::unique_ptr<DebugPrims>						debugPrims;
		std::unique_ptr<ImageComposer>					imageComposer;
		Renderer*										renderer;
	};

	typedef std::shared_ptr<Screen> ScreenPtr;
}
#endif