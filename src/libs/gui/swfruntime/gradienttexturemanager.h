// 
//  SwfGradientTextureManager.h
//  Projects
//  
//  Created by Deano on 2008-10-02.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_SWFRUNTIME_GRADIENTTEXTUREMANAGER_H_
#define YOLK_SWFRUNTIME_GRADIENTTEXTUREMANAGER_H_

#include "scene/texture.h"
#include "scene/rendercontext.h"

namespace Swf {
	// forward decl
	class SwfGradientFillStyle;
	class GradientTexturePage;

	class GradientTexture {
	public:
		friend class GradientTextureManager;

		GradientTexturePage*					page;
		int										pageY;
		Math::Vector2							offset;
		Math::Vector2							scale;
	private:
		GradientTexture() : page( nullptr ) {}

	};

	class GradientTexturePage {
	public:
		friend class GradientTextureManager;
		enum GradientPageType {
			Linear,
			Radial
		};
		GradientPageType 									pageType;
		GradientTexture*	 								gradients;
		int	 												usageCount;
		int 												lowFree;
		Scene::TexturePtr									page;
		Core::ScopedResourceHandle<Scene::TextureHandle>	textureHandle;
	private:
		GradientTexturePage() : lowFree(-1) {}

		~GradientTexturePage() {
			page.reset();
			textureHandle.reset();
		}
	};
	
	class GradientTextureManager {
	public:
		GradientTextureManager();
		static const int MAX_TEXTURE_PAGES = 16;

		GradientTexture* allocateLinearGradientTexture();
		GradientTexture* allocateRadialGradientTexture();
		void updateLinearGradient( Scene::RenderContext* _ctx, const GradientTexture* grad, SwfGradientFillStyle* _gradientFill );
		void updateRadialGradient( Scene::RenderContext* _ctx, const GradientTexture* grad, SwfGradientFillStyle* _gradientFill );

	private:
		GradientTexturePage* allocatePage( GradientTexturePage::GradientPageType _type );
		GradientTexturePage pages[MAX_TEXTURE_PAGES];
	};
	
} /* Swf */


#endif /* _SWFGRADIENTTEXTUREMANAGER_H_ */

