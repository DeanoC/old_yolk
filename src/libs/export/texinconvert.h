#pragma once
#ifndef YOLK_EXPORT_TEXINCONVERT_H_
#define YOLK_EXPORT_TEXINCONVERT_H_ 1

#if !defined(YOLK_EXPORT_TEXTUREIMAGE_H_)
#include "textureimage.h"
#endif

namespace Export {
	void convertInputToTextureImage(Export::BitmapInput& in);
} // endif namespace


#endif // endif include