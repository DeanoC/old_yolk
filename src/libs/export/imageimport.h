#pragma once
#ifndef YOLK_EXPORT_IMAGEIMPORT_H_
#define YOLK_EXPORT_IMAGEIMPORT_H_ 1

#if !defined(YOLK_EXPORT_TEXTUREIMAGE_H_)
#include "textureimage.h"
#endif

namespace Export {
	BitmapInput loadImage(const char* fileName);
} // endif namespace


#endif // endif include