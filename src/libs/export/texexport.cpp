
#include <iomanip>
#include "export.h"
#include "core/coreresources.h"
#include "scene/generictextureformat.h"
#include "scene/gtfcracker.h"
#include "export\textureimage.h"
#include "export\texinconvert.h"
#include "core/stb_image_write.h"
#include <ios>

#define GTF_START_MACRO static std::string GtfStrings[] = {
#define GTF_MOD_MACRO(x) #x,
#define GTF_END_MACRO };
#include "scene/generictextureformat.h"

namespace Export {

static void WriteTexture(	const TextureImage<double>& hpTexture,
							const GENERIC_TEXTURE_FORMAT outFmt,
							std::ostringstream& stream,
							bool textureIsNormalised, 
							uint32_t* debugOut = nullptr ) {
	union {
		float		f;
		struct {
			half_float::half ha, hb;
		};
		struct {
			uint16_t i16a, i16b;
		};
		uint32_t	i;
	} if2f;

	using namespace Export;
	const unsigned int outChannels = GtfFormat::getChannelCount( outFmt );
	const bool outFloat = GtfFormat::isFloat(outFmt);
	const bool isCompressed = GtfFormat::isCompressed(outFmt);

	stream << std::hex << std::setiosflags( std::ios_base::showbase );
	stream.width(8);
	// this isn't quite right technically but there are no half format with other bit depth channels
	if (outFloat && GtfFormat::getChannelBits(outFmt, 0) == 16) {
		stream << ".type u16\n";
	}else {
		stream << ".type u32\n";
	}

	// easiest is output floats
	if (outFloat == true) {
		for (unsigned int y = 0; y < hpTexture.getHeight(); ++y) {
			for (unsigned int x = 0; x < hpTexture.getWidth(); ++x) {
				for (unsigned int c = 0; c < outChannels; ++c) {
					const auto chanBits = GtfFormat::getChannelBits(outFmt, c);
					double val = 0;
					if (c < hpTexture.getChannelCount()) {
						val = hpTexture.value(c, x, y);
					}
					if (chanBits == 16) {
						if2f.ha = (float)val; // double to half
						stream << if2f.i16a; // write as hex
					} else {
						union {
							float f;
							uint32_t i;
						} t;
						t.f = (float)val;
						stream << t.i; // double to float written as hex
					}
					if ((x != (hpTexture.getWidth() - 1)) || (c != (outChannels - 1))) {
						stream << ",";
					} else {
						stream << "\n";
					}
				}
			}
		}
	} else {
		for (unsigned int y = 0; y < hpTexture.getHeight(); ++y) {
			for (unsigned int x = 0; x < hpTexture.getWidth(); ++x) {
				unsigned int accumCount = 32; // write 32 bit chunks
				uint32_t payload = 0;
				for (unsigned int c = 0; c < outChannels; ++c) {
					const auto chanBits = GtfFormat::getChannelBits(outFmt, c);
					double val = 0;
					// allow more channels than input set to 0 (TODO?)
					if (c < hpTexture.getChannelCount()) {
						val = hpTexture.value(c, x, y);
					}
					// if format is fixed point 0-1 multiply before floor
					val = val * (textureIsNormalised ? ((1 << chanBits)-1) : 1.0);

					int ival = static_cast<int>( floor(val) );
					ival &= (1 << chanBits) - 1;

					// pack channel bits into bit stream
					accumCount -= chanBits;
					CORE_ASSERT(accumCount < 32); // check for underflow
					payload |= ival << accumCount;
					const bool spaceLeft = (c < outChannels - 1) &&
											(accumCount >= GtfFormat::getChannelBits(outFmt, c + 1));
					if (spaceLeft == false) {
						// dump channels in text format
						stream << payload;
						if ((x != (hpTexture.getWidth() - 1)) || (c != (outChannels - 1))) {
							stream << ",";
						}
						else {
							stream << "\n";
						}
						if (debugOut != nullptr){
							*debugOut++ = payload;
						}
						accumCount = 32;
						payload = 0;
					}
				}
			}
		}
	}

	//reset stream to defaults
	stream.width( 6 );
	stream << std::defaultfloat;
	stream << std::hex << std::resetiosflags(std::ios_base::showbase);
}

void SaveTexture( const TextureExport& itex, const Core::FilePath outFilename ) {
	using namespace Core;

	TextureExport tex = itex;
	auto fileName = outFilename.BaseName();

	//--------------------------------
	// Write out texture
	//--------------------------------
	std::ostringstream outStream;

	// produce a texture header 
	outStream << "// texture file\n";
	outStream << ".type u32\n";
	outStream << "// Start texture header" << "\n";
	outStream << RESOURCE_NAME('T','X','T','R') << "\t\t\t\t// TXTR\n";	// magic
	outStream << 1 << "\t\t\t\t// version\n";
	outStream << tex.outFormat << "\t\t\t\t// format " << GtfStrings[tex.outFormat] << "\n";
	outStream << tex.outFlags << "\t\t\t\t // flags = " <<
					((tex.outFlags & TextureExport::TE_CUBEMAP ) ? "TE_CUBEMAP" : "0") << "\n";
	outStream << tex.outWidth << "\t\t\t\t// width\n";
	outStream << tex.outHeight << "\t\t\t\t// height\n";
	outStream << tex.outDepth << "\t\t\t\t// depth\n";
	outStream << tex.outSlices << "\t\t\t\t// slices \n";
	outStream << tex.outMipMapCount << "\t\t\t\t// mipmap count\n";
	outStream << "endLabel - beginLabel\t\t\t\t // total size\n";

	outStream << ".align 8\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";


	for( size_t i = 0; i < tex.bitmaps.size(); ++i ) {
		std::unique_ptr<uint32_t> debugOut;
		const bool doDebugPnt = (tex.outFormat == GTF_SRGB8_ALPHA8 && i == 0) &&
#if defined(_DEBUG)
			true;
#else
			false;
#endif
		// convert into a texture image if not already done
		if (!tex.bitmaps[i].textureImage) {
			convertInputToTextureImage(tex.bitmaps[i]);
		}

		std::shared_ptr<TextureImage<double>> hpTexture( tex.bitmaps[i].textureImage );
		if (tex.bitmaps[i].width != tex.outWidth ||
			tex.bitmaps[i].height != tex.outHeight) {
			hpTexture = hpTexture->resizeFilter(tex.outWidth, tex.outHeight); // TODO depth
		}

		if (doDebugPnt) {
			debugOut.reset(CORE_NEW uint32_t[hpTexture->getWidth() * hpTexture->getHeight() * hpTexture->getDepth()]);
		}

		WriteTexture(*hpTexture.get(), tex.outFormat, outStream, !!(tex.outFlags & TextureExport::TE_NORMALISED), debugOut.get());

		if (doDebugPnt) {
			auto pngpath = fileName.ReplaceExtension(".png");
			stbi_write_png(pngpath.value().c_str(), hpTexture->getWidth(), hpTexture->getHeight(), 4, debugOut.get(), 0);
		}
	}

	outStream << "endLabel:\n";

//#if defined(_DEBUG)
	auto textpath = fileName.ReplaceExtension(".txrtxt");
	std::ofstream foutStream;
	foutStream.open( textpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();

//#endif

	auto path = fileName.ReplaceExtension(".txr");
	foutStream.open( path.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();
}
void SaveTextureToPNG(const TextureExport& itex, const Core::FilePath outFilename) {
	auto fileName = outFilename.BaseName();
	TextureExport tex = itex;

	//--------------------------------
	// dummy write out texture not actually used
	//--------------------------------
	std::ostringstream outStream;

	for (size_t i = 0; i < tex.bitmaps.size(); ++i) {
		std::unique_ptr<uint32_t> pngOut;

		convertInputToTextureImage(tex.bitmaps[i]);
		std::shared_ptr<TextureImage<double>> hpTexture( tex.bitmaps[i].textureImage );

		hpTexture = hpTexture->resizeFilter(tex.outWidth, tex.outHeight); // TODO depth

		pngOut.reset(CORE_NEW uint32_t[hpTexture->getWidth() * hpTexture->getHeight() * hpTexture->getDepth()]);

		WriteTexture(*hpTexture.get(), tex.outFormat, outStream, 
									!!(tex.outFlags & TextureExport::TE_NORMALISED), pngOut.get());

		auto pngpath = fileName.ReplaceExtension(".png");
		stbi_write_png(pngpath.value().c_str(), hpTexture->getWidth(), hpTexture->getHeight(), 4, 
									pngOut.get(), 0);
	}

}

}