#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "core/clock.h"
#include "export/edtaa3func.h"
#include "export/export.h"
#include "export/imageimport.h"
#include "core/rapidjson/document.h"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include "core/stb_image.h"
#include "do.h"

#include <codecvt>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_ADVANCES_H

// convert wstring to UTF-8 string
namespace {
	std::string wstring_to_utf8(const std::wstring& str)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(str);
	}

	std::string ReadFontTao(const std::string& filestring, std::unordered_map<uint32_t, Export::Glyph>& outGlyphs ){
		std::string outFilename;

		using namespace rapidjson;
		Document doc;
		doc.Parse<0>(filestring.c_str());

		for (Value::ConstMemberIterator val = doc.MemberBegin();
			val != doc.MemberEnd();
			++val) {
			std::string attr = val->name.GetString();
			boost::algorithm::to_lower(attr);

			// decode metadata includes original texture filename
			if (attr == "meta") {
				for (Value::ConstMemberIterator mval = val->value.MemberBegin();
					mval != val->value.MemberEnd();
					++mval) {
					std::string mattr = mval->name.GetString();
					boost::algorithm::to_lower(mattr);
					if (mattr == "image") {
						if (mval->value.IsString() == false) {
							continue;
						}
						std::string mvalstr = mval->value.GetString();
						CORE_ASSERT(outFilename.empty());
						outFilename = mvalstr;
						continue;
					}
				}
				continue;
			}

			// each frame in the textue atlas
			if (attr == "frames") {
				uint16_t count = 0;
				for (Value::ConstMemberIterator aval = val->value.MemberBegin();
							aval != val->value.MemberEnd();
							++aval) {
					std::string name = aval->name.GetString();
					size_t last_index = name.find_last_not_of("0123456789");
					std::string numString = name.substr(last_index + 1);
					int num = boost::lexical_cast<int>(numString);
					auto glyph = outGlyphs.find(num);
					glyph->second.page = 0; // TODO multiple sprite pages
					glyph->second.sprite = count++;
				}
			}
		}
		return outFilename;
	}
}

void generateSummedAreaTable1D(const edtaa3real* img, const unsigned int w, unsigned int h, edtaa3real* out) {
	// handle border (top left) with a 0.0 border

	// handle 0th row
	for (unsigned int j = 0; j < w - 1; j++) {
		out[j] = img[j];
	}

	for (unsigned int i = 1; i < h; i++) {
		out[i*w] = img[i*w]; // handle 0th col
		for (unsigned int j = 1; j < w; j++) {
			unsigned int k00 = (i - 0)*w + (j - 0);
			unsigned int k10 = (i - 1)*w + (j - 0);
			unsigned int k01 = (i - 0)*w + (j - 1);
			unsigned int k11 = (i - 1)*w + (j - 1);
			out[k00] = img[k00] + out[k10] + out[k01] + out[k11];
		}
	}
}

// TODO generate distance map at higher resolution then downsample.
void generateDistanceMapFromBitmap(const int width, const int height, const std::vector<edtaa3real>& buffer, std::vector<edtaa3real>& distance){
	typedef edtaa3real real;
	const int size = width * height;

	// data is stored 1 bit per pixel 8 bits per pixel, we now 
	// need to make distance maps, first job convert to 
	std::vector<real> fimg(width * height, 0.f);
	std::copy(buffer.begin(), buffer.end(), fimg.begin());

	// temp data, 
	// may be possible to re-use some arrays
	std::vector<real> gx(size, 0.f);
	std::vector<real> gy(size, 0.f);
	std::vector<real> inside(size, 0.f);
	std::vector<real> outside(size, 0.f);
	std::vector<int16_t> xdist(size);
	std::vector<int16_t> ydist(size);

	// compute gradient
	// TODO is there nessecary for binary input images?
	computegradient(fimg.data(), width, height, gx.data(), gy.data());
	// distance to the outside
	edtaa3(fimg.data(), gx.data(), gy.data(), width, height,
		xdist.data(), ydist.data(), outside.data());
	// clamp outside distances to 0
	std::replace_if(outside.begin(), outside.end(),
		[](const real v) -> bool { return v < 0.f; },
		0);

	// inverse pass (1 - intensity)
	std::transform(fimg.begin(), fimg.end(), fimg.begin(),
		[](const real v){ return 1 - v; });
	// memset gx and gy
	std::fill(gx.begin(), gx.end(), 0.f);
	std::fill(gy.begin(), gy.end(), 0.f);
	// compute gradient
	// TODO is this nessecary for binary input images?
	computegradient(fimg.data(), width, height, gx.data(), gy.data());
	// distance to the inside
	edtaa3(fimg.data(), gx.data(), gy.data(),
		width, height,
		xdist.data(), ydist.data(), inside.data());
	// clamp inside distance to 1.f
	std::replace_if(inside.begin(), inside.end(),
		[](const real v) -> bool { return v > 1.f; },
		1);
	// distance is the difference between outside array and inside
	std::transform(outside.begin(), outside.end(), inside.begin(), distance.begin(),
		[](const real &o, const real &i) -> real {
		return (o - i);
	});
	// compute abs(min(distance))
	real dmin = *std::min_element(distance.begin(), distance.end());
	dmin = abs(dmin);
	// clamp distance to [-min,+min]
	std::transform(distance.begin(), distance.end(), distance.begin(),
		[dmin](const real& d) -> real {
		return std::min(std::max(d, -dmin), +dmin);
	});
	// normalise distance
	std::transform(distance.begin(), distance.end(), distance.begin(),
		[dmin](const real d) -> real {
		return (d + dmin) / (2 * dmin);
	});

	/*
	// invert distance field to help shader
	std::transform(distance.begin(), distance.end(), distance.begin(),
		[](const real d) -> real {
		return 1 - d;
	});
	*/
}

void convertToTexture(	const unsigned int glyph,
						const int width, const int height, 
						const std::vector<edtaa3real>& distance,
						const Core::FilePath& fileName) {
	using namespace Export;

	TextureExport tex;
	tex.outFormat = GTF_SRGB8_ALPHA8;
	tex.outFlags = TextureExport::TE_NORMALISED;
	tex.outWidth = width;
	tex.outHeight = height;
	tex.outDepth = 1;
	tex.outSlices = 1;
	tex.outMipMapCount = 1;

	BitmapInput bi;
	bi.flags = BitmapInput::BI_DOUBLE;
	bi.width = width;
	bi.height = height;
	bi.data = (uint8_t const*)distance.data();
	bi.channels = 1;
	tex.bitmaps.push_back(bi);

	Export::SaveTextureToPNG(tex, fileName.InsertBeforeExtension(boost::lexical_cast<std::string>(glyph)));
}

void convertToSummedAreaTableTexture(const unsigned int glyph,
	const int width, const int height,
	const std::vector<edtaa3real>& distance,
	const Core::FilePath& fileName) {
	using namespace Export;

	TextureExport tex;
	tex.outFormat = GTF_R32F;
	tex.outFlags = TextureExport::TE_NORMALISED;
	tex.outWidth = width;
	tex.outHeight = height;
	tex.outDepth = 1;
	tex.outSlices = 1;
	tex.outMipMapCount = 1;

	BitmapInput bi;
	bi.flags = BitmapInput::BI_FLOAT;
	bi.width = width;
	bi.height = height;
	bi.data = (uint8_t const*)distance.data();
	bi.channels = 1;
	tex.bitmaps.push_back(bi);

	Export::SaveTextureToPNG(tex, fileName.InsertBeforeExtension(boost::lexical_cast<std::string>(glyph)));
}


void DoTrueTypeFont(const Core::FilePath& inFullPath, const Core::FilePath& outPath, const PACKING_ORDER runOrder) {

	FT_Library    library;
	FT_Face       face;

	FT_GlyphSlot  slot;
	FT_Error      error;

	error = FT_Init_FreeType(&library);              /* initialize library */
	if (error != 0) {
		LOG(INFO) << "FreeType Init error";
		return;
	}

	LOG(INFO) << "Input Path : " << inFullPath.DirName().value().c_str() << "\n";

	boost::filesystem::current_path(inFullPath.DirName().value());

	Core::FilePath fileName = inFullPath.BaseName();

	Core::MemFile fio(fileName.value().c_str());
	if (!fio.isValid()) {
		LOG(INFO) << fileName.value() << " not found\n";
		return;
	}

	error = FT_New_Memory_Face(library, fio.getBuffer(), (FT_Long)fio.bytesLeft(), 0, &face);
	if (error != 0) {
		LOG(INFO) << "FreeType FT_New_Memory_Face error";
		return;
	}

	// h is the height in pixels of each glyph (<< 6 accounts for FT being fixed point with 6 fractional bits)
	// we want a 'standard' 18pt glyph to be stored at highest dpi we can get for the texture we are using
	// 72pt = 1 inch high therefore 18pt = 1/4inch high
	// we want a single 2Kx2K 8 bit distance field for the font. For latin lets have max 256 glyphs
	// so roughly 16x16 leading us to 128x128pixels per inch with a dpi of 128 
	// therefore 18pt AKA 1/4inch @ 128dpi = 2K^2 page of 16x16 glyphs ~32pixels for 18pt font( before padding)

	// scale not currently supported in distance before pack run order
	const int scale = (runOrder == DISTANCE_BEFORE_PACK) ? 1 : 4;
	const int h = 72 * scale;
	const int dpi = 256;
	error = FT_Set_Char_Size(face, h << 6, 0, dpi, 0);
	if (error != 0) {
		LOG(INFO) << "FreeType FT_Set_Char_Size error";
		return;
	}

	slot = face->glyph;

	// output directory for character png's
//	const Core::FilePath tmpDir( wstring_to_utf8(boost::filesystem::temp_directory_path().native()) );
	const Core::FilePath tmpDir = outPath.DirName().Append("Textures").Append("tmp");

	LOG(INFO) << "Temp Path : " << tmpDir.value() << "\n";

//#define TEST_WITH_EXISTING_TEXTURES
// testing with existing texture TEST_WITH_EXISTING_TEXTURES is defined
#if !defined(TEST_WITH_EXISTING_TEXTURES)
	// in debug or in case of crash, clean up any tmp folder
	if (boost::filesystem::exists(tmpDir.value()) ) {
		boost::system::error_code ec; // hide the error if for some reason the remove above still failed to work
		boost::filesystem::remove_all(tmpDir.value(),ec);
		Core::Clock::sleep(0.5f); // create fails if delete hasn't finished...
	}

	boost::system::error_code ec; // hide the error if for some reason the remove above still failed to work
	boost::filesystem::create_directory(tmpDir.value(),ec);

	boost::filesystem::current_path(tmpDir.value());

	// fast search glyphs by unicode code point
	std::unordered_map<uint32_t, Export::Glyph> outGlyphs;
	for (unsigned int ch = 0; ch < 256; ++ch) {

		//Load the Glyph for our character.
		if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_TARGET_NORMAL ))
			throw std::runtime_error("FT_Load_Glyph failed");

		//Move the face's glyph into a Glyph object.
		FT_Glyph glyph;
		if (FT_Get_Glyph(face->glyph, &glyph))
			throw std::runtime_error("FT_Get_Glyph failed");
		
		if (glyph->format != FT_GLYPH_FORMAT_BITMAP) {
			//Convert the glyph to a bitmap.
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, 1);
		}

		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

		//This reference will make accessing the bitmap easier
		FT_Bitmap& bitmap = bitmap_glyph->bitmap;
		if (bitmap.width > 0) {
			const int inBitmapPitch = bitmap.pitch;
			const int inBitmapWidth = bitmap.width;
			const int inBitmapHeight = bitmap.rows;
			const int outPaddX = (runOrder == DISTANCE_BEFORE_PACK) ? 2 : 0;
			const int outPaddY = (runOrder == DISTANCE_BEFORE_PACK) ? 2 : 0;
			const int outBitmapWidth = inBitmapWidth + (outPaddX * 2);
			const int outBitmapHeight = inBitmapHeight + (outPaddY * 2);

			std::vector<edtaa3real> binaryMap(outBitmapWidth * outBitmapHeight, 0);

			// convert from binary to float
			switch (bitmap.pixel_mode){
			case FT_PIXEL_MODE_MONO:
				for (int y = 0; y < inBitmapHeight; ++y) {
					for (int x = 0; x < inBitmapWidth; ++x) {
						const uint8_t bite = *(bitmap.buffer + (y * inBitmapPitch) + x);

						edtaa3real* fout = binaryMap.data() +
							(((y + outPaddY) * outBitmapWidth) + ((x * 8) + outPaddX));
						for (int i = 0; i < 8; ++i){
							if (((x * 8) + i) < inBitmapWidth) {
								*(fout + i) = bite & (1 << (7 - i)) ? 1.0f : 0.0f;
							}
						}
					}
				}
				break;
			case FT_PIXEL_MODE_GRAY:
				for (int y = 0; y < inBitmapHeight; ++y) {
					for (int x = 0; x < inBitmapWidth; ++x) {
						const uint8_t bite = *(bitmap.buffer + (y * inBitmapPitch) + x);
						edtaa3real* fout = binaryMap.data() +
							(((y + outPaddY) * outBitmapWidth) + (x + outPaddX));
						*(fout) = ((edtaa3real)bite) / edtaa3real(256);
					}
				}
				break;
			default:
				CORE_ASSERT("font format not supported");
			}

			if (runOrder == DISTANCE_BEFORE_PACK) {
				std::vector<edtaa3real> distanceMap(outBitmapWidth * outBitmapHeight, 0);
				generateDistanceMapFromBitmap(outBitmapWidth, outBitmapHeight, binaryMap, distanceMap);
				binaryMap.resize(0); binaryMap.shrink_to_fit();
				convertToTexture(ch, outBitmapWidth, outBitmapHeight, distanceMap, fileName);
			} else {
				convertToTexture(ch, outBitmapWidth, outBitmapHeight, binaryMap, fileName);
			}

			// store glyph data
			Export::Glyph outGlyph;
			outGlyph.unicode = ch;
			outGlyph.width = (uint16_t)(face->glyph->metrics.width / 64) + (outPaddX * 2);
			outGlyph.height = (uint16_t)(face->glyph->metrics.height / 64) + (outPaddY * 2);
			outGlyph.offsetX = (int16_t)(slot->bitmap_left + (face->glyph->metrics.horiBearingX / 64));// +outPaddX;
			outGlyph.offsetY = (int16_t)(slot->bitmap_top + (face->glyph->metrics.horiBearingY / 64));// +outPaddY;

			// get advance info without hinting
			if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_RENDER | FT_LOAD_NO_HINTING))
				throw std::runtime_error("FT_Load_Glyph failed");

			outGlyph.advanceX = (int16_t)(face->glyph->metrics.horiAdvance >> 6);
			outGlyph.advanceY = (int16_t)0;

			outGlyphs[ch] = outGlyph;
		}

		FT_Done_Glyph(glyph);
	}

	Export::FontMetrics fontMetrics;
	fontMetrics.ascender = ((face->ascender >> 6) * dpi) / h / scale;
	fontMetrics.descender = ((face->descender >> 6) * dpi) / h / scale;
	fontMetrics.height = ((face->height >> 6) * dpi) / h / scale;
	fontMetrics.dpi = dpi;

	FT_Done_Face(face);
	FT_Done_FreeType(library);
#endif // end TEST_WITH_EXISTING_TEXTURES

	using namespace boost::filesystem;
	current_path(tmpDir.value());

	if (!boost::filesystem::exists("packer")) {
		boost::filesystem::create_directory("packer");
	}

	// now call texture packer to convert it into texture pages
	// TODO make this non filesystem setup dependant and non windows...
	std::string tpCmdLine = "\"C:\\Program Files\\CodeAndWeb\\TexturePacker\\bin\\texturepacker\"";
	tpCmdLine += " ../../../source/Templates/font_template.tps";
	const std::string fontName = outPath.BaseName().RemoveExtension().value();
	const std::string taoName = fontName + "{n}.tao";
	tpCmdLine += " --data packer/" + taoName;
	tpCmdLine += " --texture-format png";
	tpCmdLine += " --trim-sprite-names";
	tpCmdLine += " --sheet packer/" + outPath.BaseName().RemoveExtension().value() + "{n}.png";
	tpCmdLine += " --shape-padding " + boost::lexical_cast<std::string>(scale * 4);
	tpCmdLine += " .";

	system( tpCmdLine.c_str() );
	// currently we only support a single texture atlas per font... fix here if this becomes
	// a problem (combine tao's?)

	std::string	textureAtlasPath;

	// TODO export single channel format
	// TODO texture compression
	std::for_each(directory_iterator( "packer/"), directory_iterator(),
		[=, &textureAtlasPath](const boost::filesystem::path& path) {
			if (path.extension() == ".tao") {
				if (runOrder == PACK_BEFORE_DISTANCE) {

					// TODO support multiple texture pages
					// we stop texture atlas from converting the textures and do it ourself from our fancy
					// distance texture
					const auto p = Core::FilePath(wstring_to_utf8(path.wstring()));
					const auto filenames = DoTextureAtlas(p, outPath, false);

					for (auto fi : filenames) {
						// load in the packed 'binary' image to run distance and optionally
						// summed area table
						current_path(tmpDir.value());
						auto img = Export::loadImage(p.DirName().Append(fi).value().c_str());
						CORE_ASSERT(img.textureImage);
						img.textureImage = img.textureImage->changeChannelCount(1);

						std::vector<edtaa3real> distanceMap(img.width * img.height, 0);
						generateDistanceMapFromBitmap(img.width, img.height, img.textureImage->getData(), distanceMap);

						/* TODO
						std::vector<edtaa3real> summedAreaTable(img.width * img.height, 0.f);
						generateSummedAreaTable1D(distanceMap.data(), img.width, img.height, summedAreaTable.data());
						std::copy(summedAreaTable.begin(), summedAreaTable.end(), img.textureImage->getData().begin());
						*/

						std::copy(distanceMap.begin(), distanceMap.end(), img.textureImage->getData().begin());
						distanceMap.resize(0); distanceMap.shrink_to_fit();

						Export::TextureExport texp;
						memset(&texp, 0, sizeof(Export::TextureExport));
						texp.outWidth = img.width / scale;
						texp.outHeight = img.height / scale;
						texp.outMipMapCount = 1;
						texp.outSlices = 1;
						texp.outDepth = 1;
						texp.outFormat = GTF_R32F;
						Export::BitmapInput bi;
						bi.width = img.width;
						bi.height = img.height;
						bi.channels = 1;
						bi.flags = Export::BitmapInput::BI_DOUBLE;
						bi.textureImage = img.textureImage;
						texp.bitmaps.push_back(bi);

						DoTexture(texp, Core::FilePath(fi), outPath.DirName().Append(Core::FilePath(fi).BaseName()));
					}

				} else {
					DoTextureAtlas(Core::FilePath(wstring_to_utf8(path.wstring())), outPath, true );
				}


				textureAtlasPath = wstring_to_utf8(path.wstring());
			}
		}
	);
	current_path(tmpDir.value());
	Core::MemFile file;
	Core::FilePath toaPath = Core::FilePath(textureAtlasPath).ReplaceExtension(".tao");
	bool ok = file.loadTextFile(toaPath.value().c_str());
	auto filestring = std::string((char*)file.takeBufferOwnership());
	file.close();

#if !defined(TEST_WITH_EXISTING_TEXTURES)

	for (auto& gl : outGlyphs) {
		gl.second.width /= scale;
		gl.second.height /= scale;
		gl.second.advanceX /= scale;
		gl.second.advanceY /= scale;
		gl.second.offsetX /= scale;
		gl.second.offsetY /= scale;
	}
	ReadFontTao(filestring, outGlyphs);
	assert(!fontName.empty());
	std::vector<Export::Glyph> vecGlyphs(outGlyphs.size());
	std::transform(outGlyphs.cbegin(), outGlyphs.cend(), vecGlyphs.begin(), 
		[](const std::pair<uint32_t, Export::Glyph>& in) {
			return in.second;
		}
	);
	// sort into unicode code point order for fast look up at real-time
	std::sort(vecGlyphs.begin(), vecGlyphs.end(), 
		[](const Export::Glyph& a, const Export::Glyph&b){
			return a.unicode < b.unicode;
		}
	);
	SaveFont(fontName, fontMetrics, vecGlyphs, outPath);
#endif

#if !defined(_DEBUG)
	// in release cleanup tmp folder, in debug leave for examination
	if (boost::filesystem::exists(tmpDir.value())) {
		// error code version used as sometime release version fails (timing?)
		// it appears to be removing the directory that causes the fail
		// its harmless so just ignore for now
		boost::system::error_code ec;
		boost::filesystem::remove_all(tmpDir.value(),ec);
	}
#endif
}

