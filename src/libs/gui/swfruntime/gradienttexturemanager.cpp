// 
//  SwfGradientTextureManager.cpp
//  Projects
//  
//  Created by Deano on 2008-10-02.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/swfparser/SwfGradient.h"
#include "gui/swfparser/SwfFillStyle.h"
#include "scene/texture.h"
#include "utils.h"
#include "gradienttexturemanager.h"

namespace Swf {


static const int LINEAR_GRADIENT_WIDTH = 256;
static const int LINEAR_GRADIENT_HEIGHT = 2048; // 3 lines per linear gradient so 512 * 2048 * RGBA texture
static const int RADIAL_GRADIENT_WIDTH = 256; // radial are square width * width
static const int RADIAL_GRADIENT_PAGE_HEIGHT = 1024; // how big each page is


GradientTextureManager::GradientTextureManager() {
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
			using namespace Scene;

			Texture::CreationInfo ccs = Texture::TextureCtor( RCF_TEX_2D | RCF_PRG_READ, GTF_SRGB8_ALPHA8, width, height );
			page.textureHandle.reset( TextureHandle::create( "_SWF_GradientTex", &ccs, Core::RMRF_DONTCACHE ) );
			page.page = page.textureHandle.acquire();

			return &page;
		} else if (page.usageCount < maxUsageCount && page.pageType == _type) {
			return &page;
		}
	}
	return NULL;
}
GradientTexture* GradientTextureManager::allocateLinearGradientTexture() {
	GradientTexturePage* page = allocatePage( GradientTexturePage::Linear ); // lets find a free texture page
	for (; page->lowFree < LINEAR_GRADIENT_HEIGHT; ++page->lowFree) {
		GradientTexture* grad = &page->gradients[ page->lowFree ];
		if (grad->page == nullptr ) {
			grad->page = page;
			grad->offset = Math::Vector2(0, ((float)(page->lowFree*3)+1) / ((float)LINEAR_GRADIENT_HEIGHT*3));
			grad->scale = Math::Vector2(1, 0);
			grad->pageY = page->lowFree++;
			page->usageCount++;
			return grad;
		}
	}
	return nullptr;
}

GradientTexture* GradientTextureManager::allocateRadialGradientTexture() {
	GradientTexturePage* page = allocatePage( GradientTexturePage::Radial ); // lets find a free texture page
	for (; page->lowFree < LINEAR_GRADIENT_HEIGHT; ++page->lowFree) {
		GradientTexture* grad = &page->gradients[ page->lowFree ];
		if (grad->page == nullptr ) {
			grad->page = page;
			grad->offset = Math::Vector2(0, ((float)(page->lowFree * RADIAL_GRADIENT_WIDTH)) / ((float)RADIAL_GRADIENT_PAGE_HEIGHT));
			grad->scale = Math::Vector2(1, (float) RADIAL_GRADIENT_WIDTH / (float) RADIAL_GRADIENT_PAGE_HEIGHT);
			grad->pageY = page->lowFree++;
			page->usageCount++;
			return grad;
		}
	}
	return nullptr;
}

void GradientTextureManager::updateLinearGradient( Scene::RenderContext* _ctx, const GradientTexture* grad, SwfGradientFillStyle* _gradientFill ) {
	// generate the gradient and upload to GPU
	uint32_t colours[LINEAR_GRADIENT_WIDTH*3];
	for (int i = 0; i < LINEAR_GRADIENT_WIDTH; i++) {
		float ratio = ((float)i) / (float)LINEAR_GRADIENT_WIDTH;
		ratio = std::min(ratio, 1.0f);
		ratio = std::max(ratio, 0.0f);
		colours[i] = _gradientFill->Gradient()->SampleGradient(ratio).ToPackedABGR();
		colours[i + 1*LINEAR_GRADIENT_WIDTH] = colours[i];
		colours[i + 2*LINEAR_GRADIENT_WIDTH] = colours[i];
	}

	Scene::ResourceMapAccess access;
	access.data = colours;
	access.widthStride = sizeof(uint32_t) * LINEAR_GRADIENT_WIDTH;
	access.depthStride = 0;

	grad->page->page->update( _ctx, 0, 0,  0, grad->pageY*3, 0, LINEAR_GRADIENT_WIDTH, 3, 1, &access );
}

void GradientTextureManager::updateRadialGradient( Scene::RenderContext* _ctx, const GradientTexture* grad, SwfGradientFillStyle* _gradientFill ) {
	// generate the gradient and upload to GPU
	uint32_t colours[RADIAL_GRADIENT_WIDTH * RADIAL_GRADIENT_WIDTH];
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
	Scene::ResourceMapAccess access;
	access.data = colours;
	access.widthStride = sizeof(uint32_t) * RADIAL_GRADIENT_WIDTH;
	access.depthStride = 0;

	grad->page->page->update( _ctx, 0, 0,  0, grad->pageY, 0,  RADIAL_GRADIENT_WIDTH, RADIAL_GRADIENT_WIDTH, 1,  &access );		
}

} /* Swf */
