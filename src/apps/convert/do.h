#pragma once
#ifndef YOLK_CONVERT_DO_H_
#define YOLK_CONVERT_DO_H_ 1

namespace Core {
	class FilePath;
}

// to order options, we can pack after distance calcultion OR pack as greyscale/binary then do distance
// originally I did distance before pack BUT that makes adding summed area hard (as we need to use floats
// textures). However it may mean distance 'leaks' from one glyph to another...
// so proving an option
enum PACKING_ORDER {
	DISTANCE_BEFORE_PACK,
	PACK_BEFORE_DISTANCE
};

void DoTexture(const Export::TextureExport& tex, const Core::FilePath& inFullPath, const Core::FilePath& outPath);
void DoTexture(const Core::FilePath& inPath, const Core::FilePath& outPath);
std::vector<std::string> DoTextureAtlas(const Core::FilePath& inPath, const Core::FilePath& outPath, bool doTextures = true);
void DoTrueTypeFont(const Core::FilePath& inPath, const Core::FilePath& outPath, const PACKING_ORDER runOrder = DISTANCE_BEFORE_PACK);

#endif