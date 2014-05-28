#include "export.h"

namespace Export {

	bool SaveFont(	const std::string& textureAtlas,
					const Export::FontMetrics& fontMetrics,
					const std::vector<Glyph>& glyphs,
					const Core::FilePath pOutFilename ) {
	std::ostringstream outStream;
	std::map<std::string, std::string> stringTable;

	struct ExportRange {
		uint32_t rangeStart;
		uint32_t count;
		uint32_t indexStart;
	};

	std::vector<ExportRange> ranges;
	ranges.push_back( ExportRange{ glyphs[0].unicode, 0, glyphs[0].sprite } );
	auto curRange = ranges.begin();
	size_t rangeIndex = 0;
	for (auto sit = glyphs.begin(); sit < glyphs.end(); ++sit ) {
		if (sit->unicode == curRange->rangeStart + curRange->count) {
			curRange->count++;
		} else {
			CORE_ASSERT(sit->unicode > curRange->rangeStart + curRange->count);
			ranges.push_back(ExportRange{ sit->unicode, 1, (uint32_t)std::distance(glyphs.begin(), sit) });
			rangeIndex++;
			curRange = ranges.begin() + rangeIndex;
		}
	}


	stringTable["TextureAtlas:"] = textureAtlas;

	// produce a font header 
	outStream << "// font file\n";
	outStream << ".type u32\n";
	outStream << ".align 8\n";
	outStream << "// Start font Header" << "\n";
	outStream << RESOURCE_NAME('F','O','N','T') << "\t\t\t\t// FONT\n";		// magic
	outStream << "endLabel - beginLabel\t\t\t\t// data block size\n";		// size
	outStream << "(u16)" << (uint16_t) ranges.size() << "\t\t\t\t// no. glyph range\n";	// number of ranges
	outStream << "(u8)" << 2 << "\t\t\t\t// version\n";						// version
	outStream << "(u8) 0\t\t\t\t// padd\n";
	outStream << "TextureAtlas - beginLabel\t\t\t\t //texture atlas name offset\n";
	outStream << "RangeTableLabel - beginLabel\t\t\t\t  //range table offset\n";
	outStream << "GlyphTableLabel - beginLabel\t\t\t\t  //glyph table offset\n";
	outStream << "(u16)" << fontMetrics.dpi << "\t\t\t\t // dpi\n";
	outStream << "(s16)" << fontMetrics.ascender << "\t\t\t\t // ascender\n";
	outStream << "(s16)" << fontMetrics.descender << "\t\t\t\t // descender\n";
	outStream << "(u16)" << fontMetrics.height << "\t\t\t\t // height\n";

	outStream << ".align 8\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";

	// currently just one range 0 -256
	// TODO CJK support
	outStream << "RangeTableLabel:\n";
	outStream << "//--------------------RANGES-----------------------\n";
	for (auto r : ranges) {
		const uint32_t rangeStart = r.rangeStart;
		const uint32_t rangeEnd = rangeStart + r.count;
		const uint32_t indexStart = r.indexStart;
		outStream << "(u32) " << rangeStart << "\t\t\t\t// first codepoint in this range\n";
		outStream << "(u32) " << rangeEnd << "\t\t\t\t// last codepoint in this range\n";
		outStream << "(u32) " << indexStart << "\t\t\t\t //start index into glyph table\n\n";
	}

	outStream << ".align 4\n";
	outStream << "GlyphTableLabel:\n";
	outStream << "//-----------------GLYPHS-----------------------\n";
	for (const auto sit : glyphs) {
		outStream << "(u32)" << sit.unicode << "\t\t\t\t// codepoint\n";
		outStream << "(u16)" << sit.sprite << "\t\t\t\t// sprite\n";
		outStream << "(u8)" << (uint32_t)sit.page << ", (u8)0 \t\t\t\t// page, dummy\n";
		outStream << "(u16)" << sit.width << "\t\t\t\t// width\n";
		outStream << "(u16)" << sit.height << "\t\t\t\t// height\n";
		outStream << "(u16)" << sit.offsetX << "\t\t\t\t// offsetX\n";
		outStream << "(u16)" << sit.offsetY << "\t\t\t\t// offsetY\n";
		outStream << "(u16)" << sit.advanceX << "\t\t\t\t// advanceX\n";
		outStream << "(u16)" << sit.advanceY << "\t\t\t\t// advanceY\n\n";
	}

	outStream << ".align 4\n";
	outStream << "//---------------------------------------------\n";
	outStream << "// string table start\n";
	// now output the data tables
	for( const auto stIt : stringTable ) {
		outStream << stIt.first << "\n" << "\"" << stIt.second << "\\0\"" << "\n";
	}
	outStream << "endLabel:\n";

	// add a Manifest folder to the path
	auto filedir = pOutFilename.DirName();
	filedir = filedir.Append( "ui" );
	filedir = filedir.Append( pOutFilename.BaseName() );

//#if defined(_DEBUG)
	auto textpath = filedir.ReplaceExtension( ".fnttxt" );
	std::ofstream foutStream;
	foutStream.open( textpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto path = filedir.ReplaceExtension( ".fnt" );
	foutStream.open( path.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();

	return true;
}

}