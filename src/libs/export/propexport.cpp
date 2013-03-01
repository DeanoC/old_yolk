#include "export.h"
#include "boost/lexical_cast.hpp"
#include "core/coreresources.h"
#include "meshmod/property.h"
#include "meshmod/scene.h"

namespace Export {

void SavePropList( const std::vector< MeshMod::PropertyPtr >& props, const Core::FilePath outFilename ) {
	using namespace Core;
	std::ostringstream outStream;
	std::ostringstream propStream;
	std::map<std::string,std::string> stringTable;
	std::map<std::string,std::string> dataTable;

	int propCount = 0;
	for( auto it = props.cbegin(); it != props.cend(); ++it ) {
		auto prop = *it;
		int flags;
		int count;

		float f;
		Math::Vector3 mv3;
		Math::Quaternion mq;
		Math::Matrix4x4 mm4x4;
		const float* floatData = nullptr;
		uint8_t ui8;
		const uint8_t* ui8Data = nullptr;
		uint16_t ui16;
		const uint16_t* ui16Data = nullptr;
		uint32_t ui32;
		const uint32_t* ui32Data = nullptr;
		int32_t i32;
		const int32_t* i32Data = nullptr;
		uint64_t ui64;
		const uint64_t* ui64Data = nullptr;
		int64_t i64;
		const int64_t* i64Data = nullptr;
		std::string s;
		const std::string* stringData = nullptr;
		std::vector<float> vf;
		std::vector<uint8_t> vui8;
		std::vector<uint16_t> vui16;
		std::vector<uint32_t> vui32;
		std::vector<int32_t> vi32;
		std::vector<std::string> vs;


		// work out type based on what we know
		if( prop->isType<float>() ) {
			count = 1;
			f = prop->getAs<float>();
			floatData = (const float*)&f;
		} else if( prop->isType<Math::Vector3>() ) {
			count = 3;
			mv3 = prop->getAs<Math::Vector3>();
			floatData = (const float*)mv3;
		} else if( prop->isType<Math::Quaternion>() ) {
			count = 4;
			mq = prop->getAs<Math::Quaternion>();
			floatData = (const float*) mq;
		} else if( prop->isType<Math::Matrix4x4>() ) {			
			count = 16;
			mm4x4 = prop->getAs<Math::Matrix4x4>();
			floatData = (const float*) mm4x4;
		} else if( prop->isType<std::vector<float>>() ) {
			vf = prop->getAs<std::vector<float>>();
			count = vf.size();
			floatData = (const float*)&vf[0];
		} else if( prop->isType<bool>() ) {
			count = 1;
			ui8 = (uint8_t)prop->getAs<bool>();
			ui8Data = (const uint8_t*)&ui8;
		} else if( prop->isType<uint8_t>() ) {
			count = 1;
			ui8 = prop->getAs<uint8_t>();
			ui8Data = (const uint8_t*)&ui8;
		} else if( prop->isType<std::vector<uint8_t>>() ) {
			vui8 = prop->getAs<std::vector<uint8_t>>();
			count = vui8.size();
			ui8Data = (const uint8_t*)&vui8[0];
		} else if( prop->isType<uint16_t>() ) {
			count = 1;
			ui16 = prop->getAs<uint16_t>();
			ui16Data = (const uint16_t*)&ui16;
		} else if( prop->isType<std::vector<uint16_t>>() ) {
			vui16 = prop->getAs<std::vector<uint16_t>>();
			count = vui16.size();
			ui16Data = (const uint16_t*)&vui16[0];
		} else if( prop->isType<uint32_t>() ) {
			count = 1;
			ui32 = prop->getAs<uint32_t>();
			ui32Data = (const uint32_t*)&ui32;
		} else if( prop->isType<unsigned int>() ) {
			count = 1;
			ui32 = (uint32_t)prop->getAs<unsigned int>();
			ui32Data = (const uint32_t*)&ui32;
		} else if( prop->isType<std::vector<uint32_t>>() ) {
			vui32 = prop->getAs<std::vector<uint32_t>>();
			count = vui32.size();
			ui32Data = (const uint32_t*)&vui32[0];
		} else if( prop->isType<int32_t>() ) {
			count = 1;
			i32 = prop->getAs<int32_t>();
			i32Data = (const int32_t*)&i32;
		} else if( prop->isType<int>() ) {
			count = 1;
			i32 = (int32_t)prop->getAs<int>();
			i32Data = (const int32_t*)&i32;
		} else if( prop->isType<std::vector<int32_t>>() ) {
			vi32 = prop->getAs<std::vector<int32_t>>();
			count = vi32.size();
			i32Data = (const int32_t*)&vi32[0];
		} else if( prop->isType<uint64_t>() ) {
			count = 1;
			ui64 = prop->getAs<uint64_t>();
			ui64Data = (const uint64_t*)&ui64;
		} else if( prop->isType<int64_t>() ) {
			count = 1;
			i64 = prop->getAs<int64_t>();
			i64Data = (const int64_t*)&i64;
		} else if( prop->isType<std::string>() ) {
			count = 1;
			s = prop->getAs<std::string>();
			stringData = (const std::string*)&s;
		} else if( prop->isType<std::vector<std::string>>() ) {
			vs = prop->getAs<std::vector<std::string>>();
			count = vs.size();
			stringData = (const std::string*)&vs[0];
		} else if( prop->isType<const char*>() ) {
			count = 1;
			s = prop->getAs<const char*>();
			stringData = (const std::string*)&s;
		} else {
			count = 0;
		}


		if( count > 0 ) {
			std::ostringstream propName;
			flags = 0;

			if( floatData != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_FLOAT) << "\t\t\t\t\t // No Flags | BPT_FLOAT\n";
			} else if( ui8Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_UINT8) << "\t\t\t\t\t // No Flags | BPT_UINT8\n";
			} else if( ui16Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_UINT16) << "\t\t\t\t\t // No Flags | BPT_UINT16\n";
			} else if( ui32Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_UINT32) << "\t\t\t\t\t // No Flags | BPT_UINT32\n";
			} else if( i32Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_INT32) << "\t\t\t\t\t // No Flags | BPT_INT32\n";
			} else if( ui64Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_UINT64) << "\t\t\t\t\t // No Flags | BPT_UINT64\n";
			} else if( i64Data != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_INT64) << "\t\t\t\t\t // No Flags | BPT_INT64\n";
			} else if( stringData != nullptr && count > 0) {
				propStream << "(u8) " << ((flags << 3) | BinProperty::BPT_STRING) << "\t\t\t\t\t // No Flags | BPT_STRING\n";
			}

			propStream << ".align 4\n";

			propStream << count - 1 << "\t\t\t\t\t // count : " << count << "\n";
			propName <<  "prop" << propCount;
			
			propStream << ".align 8\n";
			propStream << "0, " << propName.str() + "_Name" << " - beginLabel" << "\t\t\t\t\t// name\n";
			propStream << "0, " << propName.str() + "_Data"<< " - beginLabel" << "\t\t\t\t\t// data\n";
			stringTable[ propName.str() + "_Name:" ] = prop->getName();
			std::string dataString;
			for( int i = 0; i < count; i++ ) {
				if( floatData != nullptr) {
					if( i == 0 ) {
						dataString += ".type float\n";
					}
					dataString += boost::lexical_cast<std::string>( floatData[i] );
				} else if( ui8Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u8\n";
					}
					dataString += boost::lexical_cast<std::string>( (int) ui8Data[i] );
				} else if( ui16Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u16\n";
					}
					dataString += boost::lexical_cast<std::string>( (int) ui16Data[i] );
				} else if( ui32Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u32\n";
					}
					dataString += boost::lexical_cast<std::string>( ui32Data[i] );
				} else if( i32Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u32\n";
					}
					dataString += boost::lexical_cast<std::string>( *(uint32_t*)&i32Data[i] );
				} else if( ui64Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u64\n";
					}
					dataString += boost::lexical_cast<std::string>( ui64Data[i] );
				} else if( i64Data != nullptr) {
					if( i == 0 ) {
						dataString += ".type u64\n";
					}
					dataString += boost::lexical_cast<std::string>( i64Data[i] );
				} else if( stringData != nullptr) {
					std::ostringstream stringer;
					stringer << "\"" << stringData[i] << "\\0\"";
					dataString += stringer.str();
				}

				if( i < count-1) {
					dataString += ", ";
				}
			}
			dataTable[ propName.str() + "_Data:" ] = dataString;
			propCount++;
		}
	}

	// produce a prop header 
	outStream << "// prop file\n";
	outStream << ".type u32\n";
	outStream << "// Start binary property Header" << "\n";
	outStream << BinPropertyType << "\t\t\t\t// PROP\n";	// magic
	outStream << "(u16)" << 2 << "\t\t\t\t// version\n";		// version
	outStream << "(u16)" << propCount << "\t\t\t\t// " << propCount << " binary props from " << props.size() << " src props\n";
	outStream << "endLabel - beginLabel\t\t\t\t // total size\n";

	outStream << ".align 8\n";
	outStream << "//---------------------------------------------\n";
	outStream << "beginLabel:\n";

	outStream << propStream.str();

	outStream << "//---------------------------------------------\n";
	outStream << "// data table start\n";
	// now output the string tables
	std::map<std::string,std::string>::const_iterator dtIt = dataTable.begin();
	while( dtIt != dataTable.end() ) {
		outStream << dtIt->first << "\n" << dtIt->second << "\n";
		++dtIt;;
	}

	outStream << ".type u32\n";

	outStream << "//---------------------------------------------\n";
	outStream << "// string table start\n";
	// now output the string tables
	std::map<std::string,std::string>::const_iterator stIt = stringTable.begin();
	while( stIt != stringTable.end() ) {
		outStream << stIt->first << "\n" << "\"" << stIt->second << "\\0\"" << "\n";
		++stIt;
	}

	outStream << "endLabel:\n";

	// add a Manifest folder to the path
	auto filedir = outFilename.DirName();
	filedir = filedir.Append( "Properties" );

	std::string fileName( outFilename.BaseName().value() );
	std::replace( fileName.begin(), fileName.end(), ' ', '_' );

	filedir = filedir.Append( fileName );

//#if defined(_DEBUG)
	auto mantextpath = filedir.ReplaceExtension( ".prptxt" );
	std::ofstream foutStream;
	foutStream.open( mantextpath.value().c_str() );
	foutStream << outStream.str();
	foutStream.close();
//#endif

	auto manpath = filedir.ReplaceExtension( ".prp" );
	foutStream.open( manpath.value().c_str(), std::ios_base::binary | std::ios_base::out );
	Binify( outStream.str(), foutStream );
	foutStream.close();
}

bool SaveProps( MeshMod::ScenePtr scene,
				Core::ResourceManifestEntryVector& manifest,
                const Core::FilePath outFilename ) {

	// collect nodes from stack to linear array
	std::vector< MeshMod::SceneNodePtr > nodeArray;
	std::stack< MeshMod::SceneNodePtr > nodes;
	int nodeCounter = 0;

	for( auto nodeIt = scene->sceneNodes.begin(); nodeIt != scene->sceneNodes.end(); ++nodeIt ) {
		nodes.push( *nodeIt );
	}

	while( !nodes.empty() ) {
		MeshMod::SceneNodePtr n = nodes.top();
		nodes.pop(); ++nodeCounter;
		nodeArray.push_back( n );
		for( uint32_t i=0;i < n->getChildCount();++i ) {
			nodes.push( n->getChild(i) );
		}
	}

	// write out the scene properties for the environment
	if( !scene->properties.empty() ) {
		SavePropList( scene->properties, outFilename );
		manifest.push_back( Core::ResourceManifestEntry( Core::BinPropertyType, outFilename.BaseName().RemoveExtension().value() ) );
	}

	// write out via flattened node array, per node properties
	for( auto it = nodeArray.cbegin(); it != nodeArray.cend(); ++it ) {
		if( !(*it)->properties.empty() ) {
			auto path = Core::FilePath( outFilename.RemoveExtension().value() + "_" + (*it)->name );
			SavePropList( (*it)->properties, path );
			manifest.push_back( Core::ResourceManifestEntry( Core::BinPropertyType, path.BaseName().RemoveExtension().value() ) );
		}
	}

	return true;

}


}