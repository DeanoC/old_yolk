#include "core/core.h"
#include "scene/generictextureformat.h"
#include "scene/gtfcracker.h"
#include "textureimage.h"
#include "core/fileio.h"
#include "core/stb_image.h"
#include <boost/filesystem.hpp>
#include "export/export.h"
#include "texinconvert.h"

//#include "imageimport.h"

namespace Export {

	BitmapInput loadImage(const char* fileName) {

		Core::MemFile fio( fileName );
		if (!fio.isValid()) {
			LOG(INFO) << fileName << " not found\n";
			BitmapInput bi;
			memset(&bi, 0, sizeof(BitmapInput));
			return bi;
		}
		stbi_io_callbacks ioCallbacks;
		ioCallbacks.read = &Core::InOutInterface::C_read;
		ioCallbacks.skip = &Core::InOutInterface::C_skip;
		ioCallbacks.eof = &Core::InOutInterface::C_eof;
		int width = 0, height = 0, components = 0;
		uint8_t* data = stbi_load_from_callbacks(
			&ioCallbacks,
			(void*)&fio.inOut(),
			&width,
			&height,
			&components, 0);

		BitmapInput bi;
		bi.flags = BitmapInput::BI_UINT8 | BitmapInput::BI_NORMALISED; // TODO hdr
		bi.width = width;
		bi.height = height;
		bi.data = data;
		switch (components) {
		case STBI_grey: bi.channels = 1; break;
		case STBI_grey_alpha: bi.channels = 2; break;
		case STBI_rgb: bi.channels = 3; break;
		case STBI_rgb_alpha:
			bi.channels = 4;
			bi.flags |= BitmapInput::BI_RGBA;
			break;
		default:
			LOG(INFO) << "Invalid component count for " << fileName << "\n";
			bi.channels = 0;
			return bi;
		}

		convertInputToTextureImage(bi);
		free(data);
		return bi;
	}
}