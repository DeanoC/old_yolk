// 
//  SwfGradientTextureManager.h
//  Projects
//  
//  Created by Deano on 2008-10-02.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFGRADIENTTEXTUREMANAGER_H_
#define _SWFGRADIENTTEXTUREMANAGER_H_

namespace Swf {
	// forward decl
	class SwfGradientFillStyle;

	class SwfGradientTexture {
	public:
		SwfGradientTexture() : textureNum(0xFFFFFFFF) {}
		uint32_t textureNum;
		Math::Vector2 offset;
		Math::Vector2 scale;
		SwfGradientFillStyle* gradientFill;
	};

	class SwfGradientTexturePage {
	public:
		enum GradientPageType {
			Linear,
			Radial
		};
		GradientPageType 				pageType;
		uint32_t 						textureNum;
		SwfGradientTexture*	 			gradients;
		int	 							usageCount;
		int 							lowFree;
	};
	
	class SwfGradientTextureManager {
	public:
		SwfGradientTextureManager();
		static const int LINEAR_GRADIENT_WIDTH = 256;
		static const int LINEAR_GRADIENT_HEIGHT = 256; // 3 lines per linear gradient so 256 * 768 * RGBA texture
		static const int RADIAL_GRADIENT_WIDTH = 64; // radial are square width * width
		static const int RADIAL_GRADIENT_PAGE_HEIGHT = 1024; // how big each page is

		static const int MAX_TEXTURE_PAGES = 16;

		SwfGradientTexture* AllocateGradient(SwfGradientFillStyle* _gradientFill);

	private:
		SwfGradientTexturePage* AllocatePage(SwfGradientTexturePage::GradientPageType _type);
		SwfGradientTexture* AllocateLinearGradient(SwfGradientFillStyle* _gradientFill);
		SwfGradientTexture* AllocateRadialGradient(SwfGradientFillStyle* _gradientFill);

		SwfGradientTexturePage pages[MAX_TEXTURE_PAGES];
	};
	
} /* Swf */


#endif /* _SWFGRADIENTTEXTUREMANAGER_H_ */

