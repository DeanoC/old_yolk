#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "edtaa3func.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

void loadTTF( const Core::FilePath& inPath ) {

	FT_Library    library;
	FT_Face       face;

	FT_GlyphSlot  slot;
	FT_Error      error;

	error = FT_Init_FreeType(&library);              /* initialize library */
	if (error != 0) {
		LOG(INFO) << "FreeType Init error";
		return;
	}

	Core::MemFile fio(inPath.value().c_str());
	if (!fio.isValid()) {
		LOG(INFO) << inPath.value() << " not found\n";
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
	// we want a single 2Kx2K 8 bit distance field for the font. For latin lets have max 128-256 glyphs
	// so roughly 8x8 leading us to 256x256pixels per glyph with a dpi of 256pixels = 1/4inch aka 1024 dpi
	int h = 256;
	error = FT_Set_Char_Size(face, h << 6, 0, 1024, 0);
	if (error != 0) {
		LOG(INFO) << "FreeType FT_Set_Char_Size error";
		return;
	}

	slot = face->glyph;

	for (unsigned char ch = 0; ch < 128; ++ch) {

		//Load the Glyph for our character.
		if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_TARGET_MONO ))
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

		std::vector<edtaa3real> distanceMap(bitmap.width * bitmap.rows);
		generateDistanceMapFromBitmap(bitmap.width, bitmap.rows, bitmap.buffer, distanceMap);

		FT_Done_Glyph(glyph);
	}

	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void generateDistanceMapFromBitmap(const int width, const int height, const uint8_t *buffer, std::vector<edtaa3real>& distance ){
	typedef edtaa3real real;
	const int size = width * height;

	// data is stored 1 bit per pixel 8 bits per pixel, we now 
	// need to make distance maps, first job convert to 
	std::vector<real> fimg(width * height, 0.f);
	prepBinaryImage(buffer, width, height, fimg.data());

	std::vector<real> gx(size, 0.f);
	std::vector<real> gy(size, 0.f);
	std::vector<real> inside(size, 0.f);
	std::vector<real> outside(size, 0.f);

	std::vector<int16_t> xdist(size);
	std::vector<int16_t> ydist(size);

	computegradient(fimg.data(), width, height, gx.data(), gy.data());
	edtaa3(fimg.data(), gx.data(), gy.data(), width, height,
				xdist.data(), ydist.data(), outside.data());
	std::replace_if(outside.begin(), outside.end(),
		[](real v) -> bool { return v < 0.f; },
		0);

	// inverse pass (1 - intensity)
	std::transform(fimg.begin(), fimg.end(), fimg.begin(),
		[](real& v){ return 1 - v; });
	std::fill(gx.begin(), gx.end(), 0.f);
	std::fill(gy.begin(), gy.end(), 0.f);
	computegradient(fimg.data(), width, height, gx.data(), gy.data());
	edtaa3(fimg.data(), gx.data(), gy.data(),
		width, height,
		xdist.data(), ydist.data(), outside.data());
	std::replace_if(inside.begin(), inside.end(),
		[](real v) -> bool { return v > 1.f; },
		1);
	std::transform(outside.begin(), outside.end(), inside.begin(), distance.begin(),
		[](const real &o, const real &i) -> real {
		return (o - i);
	});
	real dmin = *std::min_element(distance.begin(), distance.end());
	dmin = abs(dmin);
	std::transform(distance.begin(), distance.end(), distance.begin(),
		[dmin](const real& d) -> real {
		return std::max(std::min(d, -dmin), +dmin);
	});
	std::transform(distance.begin(), distance.end(), distance.begin(),
		[dmin](const real d) -> real {
		return (d + dmin) / (2 * dmin);
	});
}