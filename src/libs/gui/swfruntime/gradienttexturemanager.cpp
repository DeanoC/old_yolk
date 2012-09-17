// 
//  SwfGradientTextureManager.cpp
//  Projects
//  
//  Created by Deano on 2008-10-02.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/SwfGradient.h"
#include "gui/SwfParser/SwfFillStyle.h"
#include "utils.h"
#include "gradienttexturemanager.h"

namespace Swf {

	GradientTextureManager::GradientTextureManager() {
		uint32_t ids[MAX_TEXTURE_PAGES];
//		CALL_GL(glGenTextures(MAX_TEXTURE_PAGES, (GLuint*)ids));
		for(int i=0;i < MAX_TEXTURE_PAGES;++i){
			pages[i].textureNum = ids[i];
			pages[i].lowFree = -1;
		}
	}
	
	GradientTexturePage* GradientTextureManager::allocatePage( GradientTexturePage::GradientPageType _type ) {
		int width = LINEAR_GRADIENT_WIDTH;
		int height = LINEAR_GRADIENT_HEIGHT * 3;
		int maxUsageCount = LINEAR_GRADIENT_HEIGHT; // right for linear

		if( _type == GradientTexturePage::Radial) {
			width = RADIAL_GRADIENT_WIDTH;
			height = RADIAL_GRADIENT_PAGE_HEIGHT;
			maxUsageCount = RADIAL_GRADIENT_PAGE_HEIGHT / RADIAL_GRADIENT_WIDTH; // radial are square
		}

		for (int i = 0;i < MAX_TEXTURE_PAGES;++i) {
			GradientTexturePage& page = pages[i];
			if (page.lowFree == -1) {
				page.pageType = _type;
				page.gradients = CORE_NEW_ARRAY GradientTexture[maxUsageCount];
				page.usageCount = 0;
				page.lowFree = 0;
/*				CALL_GL(glBindTexture( GL_TEXTURE_2D, page.textureNum ));
				CALL_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
				CALL_GL(glBindTexture( GL_TEXTURE_2D, 0 ));*/
				TODO_ASSERT( false && "GL code" );
				return &page;
			} else if (page.usageCount < maxUsageCount && page.pageType == _type) {
				return &page;
			}
		}
		return NULL;
	}
	GradientTexture* GradientTextureManager::allocateLinearGradient( SwfGradientFillStyle* _gradientFill ) {
		GradientTexturePage* page = allocatePage( GradientTexturePage::Linear ); // lets find a free texture page
		for (; page->lowFree < LINEAR_GRADIENT_HEIGHT; ++page->lowFree) {
			GradientTexture* grad = &page->gradients[page->lowFree];
			if (grad->textureNum == 0xFFFFFFFF) {
				grad->gradientFill = _gradientFill;
				grad->textureNum = page->textureNum;
				// lock the texture and fill in our linear gradient
				uint32_t* colours = CORE_NEW_ARRAY uint32_t[LINEAR_GRADIENT_WIDTH*3];
				for (int i = 0; i < LINEAR_GRADIENT_WIDTH; i++) {
					float ratio = ((float)i) / (float)LINEAR_GRADIENT_WIDTH;
					ratio = std::min(ratio, 1.0f);
					ratio = std::max(ratio, 0.0f);
					colours[i] = _gradientFill->Gradient()->SampleGradient(ratio).ToPackedABGR();
					colours[i + 1*LINEAR_GRADIENT_WIDTH] = colours[i];
					colours[i + 2*LINEAR_GRADIENT_WIDTH] = colours[i];
				}
/*				CALL_GL(glBindTexture( GL_TEXTURE_2D, grad->textureNum ));
				CALL_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, page->lowFree*3, LINEAR_GRADIENT_WIDTH, 3, GL_RGBA, GL_UNSIGNED_BYTE, colours ));
				CALL_GL(glBindTexture( GL_TEXTURE_2D, 0 )); */
				TODO_ASSERT( false && "GL code" );
				CORE_DELETE_ARRAY colours;
				grad->offset = Math::Vector2(0, ((float)(page->lowFree*3)+1) / ((float)LINEAR_GRADIENT_HEIGHT*3));
				grad->scale = Math::Vector2(1, 0);
				page->lowFree++;
				page->usageCount++;
				return grad;
			}
		}
		return NULL;
	}
	GradientTexture* GradientTextureManager::allocateRadialGradient(SwfGradientFillStyle* _gradientFill) {
		GradientTexturePage* page = allocatePage( GradientTexturePage::Radial ); // lets find a free texture page
		for (; page->lowFree < RADIAL_GRADIENT_PAGE_HEIGHT / RADIAL_GRADIENT_WIDTH; ++page->lowFree) {	
			GradientTexture* grad = &page->gradients[page->lowFree];
			if (grad->textureNum == 0xFFFFFFFF) {
				grad->gradientFill = _gradientFill;
				grad->textureNum = page->textureNum;
				// lock the texture and fill in our linear gradient
				uint32_t* colours = CORE_NEW_ARRAY uint32_t[RADIAL_GRADIENT_WIDTH * RADIAL_GRADIENT_WIDTH];
				for (int j = 0; j < RADIAL_GRADIENT_WIDTH; j++) {
					for (int i = 0; i < RADIAL_GRADIENT_WIDTH; i++) {
						float radius = (RADIAL_GRADIENT_WIDTH - 1) / 2.0f;
						float y = (j - radius) / radius;
						float x = (i - radius) / radius;
						float ratio = (float) Math::LengthSquared( Math::Vector2(x,y) );
						assert( ratio > 0.0f );
						ratio = sqrtf( ratio );
						ratio = std::min(ratio, 1.0f);
						ratio = std::max(ratio, 0.0f);

						colours[ (j * RADIAL_GRADIENT_WIDTH) + i] = _gradientFill->Gradient()->SampleGradient(ratio).ToPackedABGR();
					}
				}
/*				CALL_GL(glBindTexture( GL_TEXTURE_2D, grad->textureNum ));
				CALL_GL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, page->lowFree*RADIAL_GRADIENT_WIDTH, RADIAL_GRADIENT_WIDTH, RADIAL_GRADIENT_WIDTH, GL_RGBA, GL_UNSIGNED_BYTE, colours ));
				CALL_GL(glBindTexture( GL_TEXTURE_2D, 0 ));
				*/
				TODO_ASSERT( false && "GL code" );
				CORE_DELETE_ARRAY colours;
				grad->offset = Math::Vector2(0, ((float)(page->lowFree * RADIAL_GRADIENT_WIDTH)) / ((float)RADIAL_GRADIENT_PAGE_HEIGHT));
				grad->scale = Math::Vector2(1, (float) RADIAL_GRADIENT_WIDTH / (float) RADIAL_GRADIENT_PAGE_HEIGHT);
				page->lowFree++;
				page->usageCount++;
				return grad;
			}
		}
		return NULL;
	}

	GradientTexture* GradientTextureManager::allocateGradient(SwfGradientFillStyle* _gradientFill) {
		switch (_gradientFill->fillType) {
		case LinearGradientFill:
			return allocateLinearGradient(_gradientFill);
		case RadialGradientFill:
			return allocateRadialGradient(_gradientFill);
		default:
			return NULL;
		}
	}


} /* Swf */
