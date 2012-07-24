#include "export.h"

namespace Export {

bool SaveTextureAtlas( 	const std::vector<std::string>& textures,
						const std::vector<SubTexture>& sprites,
						const Core::FilePath pOutFilename ) {
	std::ostringstream outStream;

	// produce a manifest header 
	outStream << "// man file\n";
	outStream << ".type u32\n";
	outStream << ".align 8\n";
	outStream << "// Start texture atlas Header" << "\n";
	outStream << RESOURCE_NAME('T','X','A','T') << "\t\t\t\t// TXAT\n";	// magic
	outStream << "(u8)" << 1 << "\t\t\t\t// version\n";		// version
	outStream << "(u8)" << textures.size() << "\t\t\t\t// " << textures.size() << " entries\n";		// number of textures
	outStream << "(u16)" << sprites.size() << "\t\t\t\t// " << sprites.size() << " entries\n";		// number of sprites
	outStream << "endLabel - beginLabel\t\t\t\t // total size\n";

	std::map<std::string,std::string> stringTable;

	outStream << ".align 8\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";

	auto tit = textures.begin();
	while( tit != textures.end() ) {
		outStream << "//---------------------------------------------\n";
		std::ostringstream nameMaker;
		nameMaker << "Entry_" << std::distance( textures.begin(), tit );
		std::string stabentry = nameMaker.str();
		stringTable[ stabentry + ":" ] = *tit;
		outStream << stabentry.c_str() << " - beginLabel\t\t\t\t //texture name offset\n";
		++tit;
	}
	auto sit = sprites.begin();
	while( sit != sprites.end() ) {
		outStream << "//---------------------------------------------\n";
		outStream << "(u32)" << (*sit).index << "\n";
		outStream << "(float)" << (*sit).u0 << "," << (*sit).v0 << "\n";
		outStream << "(float)" << (*sit).u1 << "," << (*sit).v1 << "\n";
		++sit;
	}

	outStream << "//---------------------------------------------\n";
	outStream << "// string table start\n";
	// now output the data tables
	std::map<std::string,std::string>::const_iterator stIt = stringTable.begin();
	while( stIt != stringTable.end() )
	{
		outStream << stIt->first << "\n" << "\"" << stIt->second << "\\0\"" << "\n";
		++stIt;;
	}
	outStream << "endLabel:\n";

	// add a Manifest folder to the path
	auto filedir = pOutFilename.DirName();
	filedir = filedir.Append( "Textures" );
	filedir = filedir.Append( pOutFilename.BaseName() );

//#if defined(_DEBUG)
	auto textpath = filedir.ReplaceExtension( ".tattxt" );
	std::ofstream foutStream;
	foutStream.open( textpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto path = filedir.ReplaceExtension( ".tat" );
	foutStream.open( path.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();

	return true;
}

}