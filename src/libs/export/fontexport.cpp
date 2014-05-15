#include "export.h"

namespace Export {

bool SaveFont(	const std::string& textureAtlas,
				const std::vector<Glyph>& glyphs,
				const Core::FilePath pOutFilename ) {
	std::ostringstream outStream;

	// produce a font header 
	outStream << "// font file\n";
	outStream << ".type u32\n";
	outStream << ".align 8\n";
	outStream << "// Start font Header" << "\n";
	outStream << RESOURCE_NAME('F','O','N','T') << "\t\t\t\t// FONT\n";		// magic
	outStream << "endLabel - beginLabel\t\t\t\t // total size\n";			// size
	outStream << "(u16)" << glyphs.size() << "\t\t\t\t// " << glyphs.size() << " entries\n";// number of glyphs
	outStream << "(u8)" << 1 << "\t\t\t\t// version\n";						// version

	std::map<std::string,std::string> stringTable;

	outStream << ".align 8\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";

	outStream << "//---------------------------------------------\n";
	std::ostringstream nameMaker;
	stringTable["TextureAtlas:"] = textureAtlas;
	outStream << "TextureAtlas - beginLabel\t\t\t\t //texture atlas name offset\n";

	auto sit = glyphs.begin();
	while (sit != glyphs.end()) {
		outStream << "//---------------------------------------------\n";
		outStream << "(u32)" << (*sit).unicode << "\n";
		outStream << "(u8)" << (uint16_t)(*sit).page << ", 0 \n";
		outStream << "(u16)" << (*sit).sprite << "\n";
		++sit;
	}

	outStream << "//---------------------------------------------\n";
	outStream << "// string table start\n";
	// now output the data tables
	std::map<std::string,std::string>::const_iterator stIt = stringTable.begin();
	while( stIt != stringTable.end() ) {
		outStream << stIt->first << "\n" << "\"" << stIt->second << "\\0\"" << "\n";
		++stIt;
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