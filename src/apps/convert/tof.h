#pragma once
#ifndef YOLK_CONVERT_TOF_H_
#define YOLK_CONVERT_TOF_H_ 1

namespace Core {
	class FilePath;
}

namespace Export {
	struct SubTexture;
}

// any Tof uint32_t attribute set to this, should use whatever the sytem
// chooses to pick
#define TOF_DEFAULT		0xFFFFFFFF

struct Tof {
	uint32_t 	format; // GENERIC_TEXTURE_FORMAT
	uint32_t 	width;
	uint32_t 	height;
	uint32_t 	depth;
	uint32_t 	arraySize;
	bool 		cubeMap;
	bool 		linear;
	std::vector< std::string > files;
};

// loads up if not possible sets up the defualt Tof
extern Tof loadTof( const Core::FilePath& inPath );
extern void loadTao( const Core::FilePath& inPath, std::vector<std::string>& outFilenames, 
													std::vector<Export::SubTexture>& outSprites );

#endif