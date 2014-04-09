#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "core/rapidjson/document.h"
#include <boost/algorithm/string.hpp> 
#include "tof.h"
#include "scene/generictextureformat.h"
#include "export/texexport.h"

#define GTF_START_MACRO static std::string GtfStrings[] = {
#define GTF_MOD_MACRO(x) #x,
#define GTF_END_MACRO };
#include "scene/generictextureformat.h"

Tof loadTof( const Core::FilePath& inPath ) {
	Tof tof;
	tof.format = TOF_DEFAULT;
	tof.width = TOF_DEFAULT;
	tof.height = TOF_DEFAULT;
	tof.depth = TOF_DEFAULT;
	tof.arraySize = TOF_DEFAULT;
	tof.cubeMap = false;
	tof.linear = false;

	Core::MemFile file;
	Core::FilePath tofPath = inPath.ReplaceExtension(".tof");
	bool ok = file.loadTextFile( tofPath.value().c_str() );

	if( ok ) {
		auto filestring = std::string( (char*)file.takeBufferOwnership() );

		using namespace rapidjson;
		Document doc;
		doc.Parse<0>( filestring.c_str() );

		for (	Value::ConstMemberIterator val = doc.MemberBegin(); 
				val != doc.MemberEnd(); 
				++val ) {
			std::string attr = val->name.GetString();
			boost::algorithm::to_lower( attr );
			if( attr == "format" ) {
				if( val->value.IsString() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				std::string valstr = val->value.GetString();
				boost::algorithm::to_upper( valstr );
				for( int i =0; i < NUM_ARRAY_ELEMENTS(GtfStrings); ++i ) {
					if( GtfStrings[i] == valstr ) {
						tof.format = i;
						break;
					}
				}
				continue;
			}
			if( attr == "width" ) {
				if( val->value.IsInt() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int width = val->value.GetInt();
				if( width > 0 ) {
					tof.width = width;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << width << "\n";
				}
				continue;
			}
			if( attr == "height" ) {
				if( val->value.IsInt() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int height = val->value.GetInt();
				if( height > 0 ) {
					tof.height = height;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << height << "\n";
				}
				continue;
			}
			if( attr == "depth" ) {
				if( val->value.IsInt() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int depth = val->value.GetInt();
				if( depth > 0 ) {
					tof.depth = depth;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << depth << "\n";
				}
				continue;
			}
			if( attr == "arraysize" ) {
				if( val->value.IsInt() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int arraysize = val->value.GetInt();
				if( arraysize > 0 ) {
					tof.arraySize = arraysize;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << arraysize << "\n";
				}
				continue;
			}
			if( attr == "filenames" || attr == "filename" ) {
				switch( val->value.GetType() ) {
				case Type::kStringType: {
						// simple single string
						std::string valstr = val->value.GetString();
						boost::algorithm::to_upper( valstr );
						tof.files.push_back( valstr );
					} break;
				case Type::kArrayType: {
						// array of filenames
						const auto& arr = val->value;
						for( SizeType i = 0; i != arr.Size(); ++i ) { 
							if( arr[i].IsString() != false ) {
								std::string avalstr = arr[i].GetString();
								boost::algorithm::to_upper( avalstr );
								tof.files.push_back( avalstr );
							}
						}
					} break;
				default: {
					LOG(INFO) << "Invalid " << attr << "type\n";
					} break;
				}
				continue;
			}
			if( attr == "cubemap" ) {
				if( val->value.IsBool() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				bool b = val->value.GetBool();
				tof.cubeMap = b;
				continue;
			}
			if( attr == "linear" ) {
				if( val->value.IsBool() == false ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				bool b = val->value.GetBool();
				tof.linear = b;
				continue;
			}

		}
		// if no filenames in the tof, just use the passed in one
		if( tof.files.empty() ) {
			tof.files.push_back( inPath.value() );
		}
	} else {
		tof.files.push_back( inPath.value() );
	}
	return tof;
}
