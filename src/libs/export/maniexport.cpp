#include "export.h"

namespace Export {

bool SaveManifest( const Core::ResourceManifestEntryVector& manifest,  const Core::FilePath pOutFilename ) {
	std::ostringstream outStream;

	// produce a manifest header 
	outStream << "// man file\n";
	outStream << ".type u32\n";
	outStream << ".align 8\n";
	outStream << "// Start mani Header" << "\n";
	outStream << Core::ManifestType << "\t\t\t\t// MANI\n";	// magic
	outStream << "(u16)" << 1 << "\t\t\t\t// version\n";		// Manifest version
	outStream << "(u16)" << manifest.size() << "\t\t\t\t// " << manifest.size() << " entries\n";		// number of entries
	outStream << "endLabel - beginLabel\t\t\t\t // total size\n";

	std::map<std::string,std::string> stringTable;

	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";

	Core::ResourceManifestEntryVector::const_iterator it = manifest.begin();
	while( it != manifest.end() ) {
		const Core::ResourceManifestEntry& maniEntry = (*it);
		outStream << "//---------------------------------------------\n";
		char* typetxt = (char*)&maniEntry.type;
		outStream << maniEntry.type << " // Entry Type: " << typetxt[0] << typetxt[1] << typetxt[2] << typetxt[3] << "\n";
		outStream << "0 \t\t// flags 0\n";
		std::ostringstream nameMaker;
		nameMaker << "Entry_" << std::distance( manifest.begin(), it );
		std::string stabentry = nameMaker.str();
		stringTable[ stabentry + ":" ] = maniEntry.filename;
		outStream << "0, " << stabentry.c_str() << " - beginLabel\t\t\t\t //resource name\n";
		outStream << "0, 0 \t\t\t\t// Ptr filled in by runtime\n";
		++it;
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
	filedir = filedir.Append( "Manifests" );
	filedir = filedir.Append( pOutFilename.BaseName() );

//#if defined(_DEBUG)
	auto mantextpath = filedir.ReplaceExtension( ".mantxt" );
	std::ofstream foutStream;
	foutStream.open( mantextpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto manpath = filedir.ReplaceExtension( ".man" );
	foutStream.open( manpath.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();

	return true;
}

}