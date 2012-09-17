// 
//  SwfGradientTextureManager.h
//  Projects
//  
//  Created by Deano on 2008-10-02.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_SWFRUNTIME_GRADIENTTEXTUREMANAGER_H_
#define YOLK_SWFRUNTIME_GRADIENTTEXTUREMANAGER_H_

namespace Swf {
	// forward decl
	class SwfGradientFillStyle;

	class GradientTexture {
	public:
		GradientTexture() : textureNum(0xFFFFFFFF) {}
		uint32_t textureNum;
		Math::Vector2 offset;
		Math::Vector2 scale;
		SwfGradientFillStyle* gradientFill;
	};

	class GradientTexturePage {
	public:
		enum GradientPageType {
			Linear,
			Radial
		};
		GradientPageType 				pageType;
		uint32_t 						textureNum;
		GradientTexture*	 			gradients;
		int	 							usageCount;
		int 							lowFree;
	};
	
	class GradientTextureManager {
	public:
		GradientTextureManager();
		static const int LINEAR_GRADIENT_WIDTH = 256;
		static const int LINEAR_GRADIENT_HEIGHT = 256; // 3 lines per linear gradient so 256 * 768 * RGBA texture
		static const int RADIAL_GRADIENT_WIDTH = 64; // radial are square width * width
		static const int RADIAL_GRADIENT_PAGE_HEIGHT = 1024; // how big each page is

		static const int MAX_TEXTURE_PAGES = 16;

		GradientTexture* allocateGradient( SwfGradientFillStyle* _gradientFill );

	private:
		GradientTexturePage* allocatePage( GradientTexturePage::GradientPageType _type );
		GradientTexture* allocateLinearGradient( SwfGradientFillStyle* _gradientFill );
		GradientTexture* allocateRadialGradient( SwfGradientFillStyle* _gradientFill );

		GradientTexturePage pages[MAX_TEXTURE_PAGES];
	};
	
} /* Swf */


#endif /* _SWFGRADIENTTEXTUREMANAGER_H_ */

