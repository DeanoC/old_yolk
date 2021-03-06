#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/stb_image.h"
#include <boost/filesystem.hpp>
#include "export/export.h"
#include "tof.h"
#include "do.h"

void DoTexture(const Core::FilePath& inFullPath, const Core::FilePath& outPath) {
	using namespace Export;
	LOG(INFO) << "Input Path : " << inFullPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path(inFullPath.DirName().value().c_str());

	Core::FilePath inPath = inFullPath.BaseName();

	Tof tof = loadTof(inPath);

	TextureExport tex;

	// load the bitmaps 
	for (auto fileIt = tof.files.cbegin();
		fileIt != tof.files.cend();
		++fileIt) {

		Core::MemFile fio(fileIt->c_str());
		if (!fio.isValid()) {
			LOG(INFO) << *fileIt << " not found\n";
			continue;
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
			LOG(INFO) << "Invalid component count for " << *fileIt << "\n";
			bi.channels = 0;
			break;
		}
		if (bi.channels > 0) {
			tex.bitmaps.push_back(bi);
		}
	}
	tex.outFormat = (GENERIC_TEXTURE_FORMAT)tof.format;
	tex.outFlags = TextureExport::TE_NORMALISED; // TODO set this properly
	tex.outWidth = tof.width;
	tex.outHeight = tof.height;
	tex.outDepth = tof.depth;
	tex.outSlices = tof.arraySize;
	tex.outMipMapCount = 1; // TODO

	// handle defaults, use first bitmap
	if (tof.format == TOF_DEFAULT) {
		uint32_t channels = tex.bitmaps[0].channels;
		switch (channels) {
		case 1: tex.outFormat = GTF_R8; break;
		case 2: tex.outFormat = GTF_RG8; break;
		case 3:
		default:
			if (tof.linear == false) {
				tex.outFormat = GTF_SRGB8;
			}
			else {
				tex.outFormat = GTF_RGB8;
			}
			break;
		case 4:
			if (tof.linear == false) {
				tex.outFormat = GTF_SRGB8_ALPHA8;
			}
			else {
				tex.outFormat = GTF_RGBA8;
			}
			break;
		}
	}
	if (tof.width == TOF_DEFAULT) {
		tex.outWidth = (int)tex.bitmaps[0].width;
	}
	if (tof.height == TOF_DEFAULT) {
		tex.outHeight = (int)tex.bitmaps[0].height;
	}
	if (tof.depth == TOF_DEFAULT && !tof.cubeMap) {
		tex.outDepth = (int)tex.bitmaps.size();
	}
	if (tof.depth != TOF_DEFAULT && tof.arraySize == TOF_DEFAULT) {
		tex.outSlices = (int)tex.bitmaps.size() / (int)tof.depth;
	}
	else {
		tex.outSlices = 1;
	}
	if (tof.cubeMap) {
		tex.outFlags |= TextureExport::TE_CUBEMAP;
		if ((tex.bitmaps.size() % 6) != 0) {
			LOG(INFO) << "Cubemap (array) missing faces for " << inPath.value() << "\n";
		}
		tex.outSlices = (int)tex.bitmaps.size(); // faces are in the arrays
		tex.outDepth = 0; // depth cubemaps make no sense
	}

	DoTexture(tex, inFullPath, outPath);
}

void DoTexture(const Export::TextureExport& tex, const Core::FilePath& inFullPath, const Core::FilePath& outPath) {
	using namespace Export;

	if (!tex.bitmaps.empty()) {
		//--------------------------------
		// add a Manifest folder to the path
		auto filedir = outPath.DirName();
		filedir = filedir.Append("Textures");

		LOG(INFO) << "Output Path : " << filedir.value().c_str() << "\n";
		boost::filesystem::current_path(filedir.value().c_str());

		Export::SaveTexture(tex, outPath.BaseName());

	} else {
		LOG(INFO) << "No bitmaps are valid for " << inFullPath.value() << "\n";
	}
}

std::vector<std::string> DoTextureAtlas(const Core::FilePath& inFullPath, const Core::FilePath& outPath, bool doTextures) {
	LOG(INFO) << "Input Path : " << inFullPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path(inFullPath.DirName().value().c_str());

	Core::FilePath inPath = inFullPath.BaseName();

	using namespace Export;
	std::vector<std::string> filenames;
	std::vector<SubTexture> sprites;

	// TODO when we handle multi-page texture atlas this will need a revisit
	//filenames.push_back(inPath.ReplaceExtension(".png").value());

	loadTao(inPath, filenames, sprites);

	if (doTextures) {
		for (auto fiIt : filenames ) {
			DoTexture(Core::FilePath(fiIt), outPath.DirName().Append(Core::FilePath(fiIt).BaseName()));
		}
	}

	LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path(outPath.DirName().value().c_str());

	SaveTextureAtlas(filenames, sprites, outPath);

	return filenames;
}
