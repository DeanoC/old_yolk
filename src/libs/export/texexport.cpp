#include "export.h"
#include "core/coreresources.h"
#include "gl/gl.h"
#include "gl/glew.h"

#include "gl/glformat_cracker.h"

#define GTF_START_MACRO static std::string GtfStrings[] = {
#define GTF_MOD_MACRO(x) #x,
#define GTF_END_MACRO };
#include "scene/generictextureformat.h"

#define GL_COMPRESSED_RGBA_S3TC_DXT1 			GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT3 			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
#define GL_COMPRESSED_RGBA_S3TC_DXT5 			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5 		GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
#define GL_COMPRESSED_RGBA_BPTC_UNORM 			GL_COMPRESSED_RGBA_BPTC_UNORM_ARB
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB

#define GL_COMPRESSED_RGB_S3TC_DXT1 			GL_COMPRESSED_RGB_S3TC_DXT1_EXT 
#define GL_COMPRESSED_SRGB_S3TC_DXT1 			GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB

#define GTF_START_MACRO static uint32_t GtfToGlFormat[] = {
#define GTF_MOD_MACRO(x) GL_##x,
#define GTF_END_MACRO };
#include "scene/generictextureformat.h"

namespace Export {

static uint32_t Up1chan8bit( const uint8_t* data) {
	return (data[0] << 24) | (data[0] << 16) | (data[0] << 8) | (data[0] << 0);
}

static uint32_t Up1chan16bit( const uint8_t* data ) {
	return (((const uint16_t*)data)[0] << 16) | (((const uint16_t*)data)[0] << 0);
}
static uint32_t Up1chan32bit( const uint8_t* data ) {
	return ((const uint32_t*)data)[0];
}

static void WriteTexture( 	const Export::BitmapInput& in, 
							const GENERIC_TEXTURE_FORMAT outFmt,
							std::ostringstream& stream  ) {
	using namespace Export;
	const uint32_t glf = GtfToGlFormat[ outFmt ];
	const unsigned int outBits = GlFormat::getBitWidth( glf );
	const unsigned int outChannels = GlFormat::getChannelCount( glf );
	stream << std::hex << std::setiosflags( std::ios_base::showbase );
	stream.width(8);
	if( outBits <= 8 ) {
		stream << ".type u8\n";
	} else if( outBits <= 16 ) {
		stream << ".type u16\n";
	} else {
		stream << ".type u32\n";		
	}
 	
	uint8_t* data = in.data;
	// not supported fully yet nor is HALF
	if( in.flags & (BitmapInput::BI_FLOAT | BitmapInput::BI_HALF) ) {
	} else {
		int dataSize = 1;
		if( in.flags & BitmapInput::BI_UINT16 ) {
			dataSize = 2;
		}

		// expand integer data to high precision
		uint32_t expandedData[4]; // 4 channels at 32 bit integer
		for( auto y = 0; y < in.height; ++y ) {
			for( auto x = 0;x < in.width; ++x ) {
				// make channel mismatch obvious
				memset( expandedData, 0xFF, sizeof(uint32_t) * 4 );
				for( auto c = 0; c < in.channels; ++c ) {
					switch( dataSize ) {
						case 4: expandedData[c] = Up1chan32bit( data++ ); break;
						case 2: expandedData[c] = Up1chan16bit( data++ ); break;
						case 1: expandedData[c] = Up1chan8bit( data++ ); break;
						default: break;
					}
				}

				if( in.flags & BitmapInput::BI_RGBA ) {
					CORE_ASSERT( in.channels == 4 );
					// swap RGBA to ABGR
					const uint32_t r = expandedData[0];
					expandedData[0] = expandedData[3]; // alpha
					expandedData[1] = expandedData[1]; // blue
					expandedData[2] = expandedData[2]; // green
					expandedData[3] = r; // red
				} else {
					// just swap order so when read by LE target comes out okay
					// TODO big endian targets
					for( auto c = 0; c < in.channels; ++c ) {
						std::swap( expandedData[c], expandedData[in.channels-c]);
					}
				}

				int accumCount = 32;
				uint32_t payload;
				// now cut back down to out precision and channels
				for( auto c = 0; c < outChannels; ++c ) {
					const auto chanBits = GlFormat::getChannelBits( glf, c );
					expandedData[c] >>= (32 - chanBits);
					expandedData[c] &= ((1 << chanBits) - 1);

					accumCount -= chanBits;
					assert( accumCount >= 0 );
					payload |= expandedData[c] << accumCount;
					if( accumCount == 0 ) {
						stream << payload;
						if( (x != (in.width - 1)) || (c != (outChannels-1)) ) {
							stream << ",";
						} else {
							stream << "\n";
						}		
						accumCount = 0;
						payload = 0;
					}
				}
			}
		}
	}
	//reset stream to defaults
	stream.width( 6 );
	stream << std::hex << std::resetiosflags( std::ios_base::showbase );
}

void SaveTexture( const TextureExport& tex, const Core::FilePath outFilename ) {
	using namespace Core;

	//--------------------------------

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

	for( int i = 0; i < tex.bitmaps.size(); ++i ) {
		WriteTexture( tex.bitmaps[i], tex.outFormat, outStream );
	}

	outStream << "endLabel:\n";
	// add a Manifest folder to the path
	auto filedir = outFilename.DirName();
	filedir = filedir.Append( "Textures" );
	filedir = filedir.Append( outFilename.BaseName() );

//#if defined(_DEBUG)
	auto textpath = filedir.ReplaceExtension( ".txrtxt" );
	std::ofstream foutStream;
	foutStream.open( textpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto path = filedir.ReplaceExtension( ".txr" );
	foutStream.open( path.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();
}

}