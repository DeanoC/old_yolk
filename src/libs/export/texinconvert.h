#pragma once
#ifndef YOLK_EXPORT_TEXINCONVERT_H_
#define YOLK_EXPORT_TEXINCONVERT_H_ 1

namespace Export {
	TextureImage<double> convertInputToTextureImage(const Export::BitmapInput& in);
} // endif namespace


#endif // endif include