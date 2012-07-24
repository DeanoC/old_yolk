#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/file_path.h"
#include "json_spirit/json_spirit_reader.h"
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

		json_spirit::Value value;
		json_spirit::read( filestring, value );
		if( value.is_null() ) {
			tof.files.push_back( inPath.value() );
			return tof;
		}

		const auto& obj = value.get_obj();
		for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
			std::string attr = val->name_;
			boost::algorithm::to_lower( attr );
			if( attr == "format" ) {
				if( val->value_.type() != json_spirit::str_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				std::string valstr = val->value_.get_str();
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
				if( val->value_.type() != json_spirit::int_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int width = val->value_.get_int();
				if( width > 0 ) {
					tof.width = width;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << width << "\n";
				}
				continue;
			}
			if( attr == "height" ) {
				if( val->value_.type() != json_spirit::int_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int height = val->value_.get_int();
				if( height > 0 ) {
					tof.height = height;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << height << "\n";
				}
				continue;
			}
			if( attr == "depth" ) {
				if( val->value_.type() != json_spirit::int_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int depth = val->value_.get_int();
				if( depth > 0 ) {
					tof.depth = depth;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << depth << "\n";
				}
				continue;
			}
			if( attr == "arraysize" ) {
				if( val->value_.type() != json_spirit::int_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				int arraysize = val->value_.get_int();
				if( arraysize > 0 ) {
					tof.arraySize = arraysize;
				} else {
					LOG(INFO) << "Invalid " << attr << " @ " << arraysize << "\n";
				}
				continue;
			}
			if( attr == "filenames" || attr == "filename" ) {
				switch( val->value_.type() ) {
					case json_spirit::str_type: {
						// simple single string
						std::string valstr = val->value_.get_str();
						boost::algorithm::to_upper( valstr );
						tof.files.push_back( valstr );
					} break;
					case json_spirit::array_type: {
						// array of filenames
						const auto& arr = val->value_.get_array();
						for( auto aval = arr.cbegin(); aval != arr.cend(); ++aval ) { 
							if( aval->type() == json_spirit::str_type ) {
								std::string avalstr = aval->get_str();
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
				if( val->value_.type() != json_spirit::bool_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				bool b = val->value_.get_bool();
				tof.cubeMap = b;
				continue;
			}
			if( attr == "linear" ) {
				if( val->value_.type() != json_spirit::bool_type ) {
					LOG(INFO) << "Invalid " << attr << "type\n";
					continue;
				}
				bool b = val->value_.get_bool();
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
		return tof;
	}
}

void loadTao( const Core::FilePath& inPath, std::vector<std::string>& outFilenames, 
											std::vector<Export::SubTexture>& outSprites ) {

	Core::MemFile file;
	Core::FilePath tofPath = inPath.ReplaceExtension(".tao");
	bool ok = file.loadTextFile( tofPath.value().c_str() );

	if( ok ) {
		auto filestring = std::string( (char*)file.takeBufferOwnership() );

		json_spirit::Value value;
		json_spirit::read( filestring, value );
		if( value.is_null() ) {
			return;
		}
		int texWidth = 0;
		int texHeight = 0;
		const auto& obj = value.get_obj();
		for( auto val = obj.cbegin(); val != obj.cend(); ++val ) {
			std::string attr = val->name_;
			boost::algorithm::to_lower( attr );

			if( attr == "meta" ) {
				const auto& mobj = val->value_.get_obj();
				for( auto mval = mobj.cbegin(); mval != mobj.cend(); ++mval ) {
					std::string mattr = mval->name_;
					boost::algorithm::to_lower( mattr );
					if( mattr == "image" ) {
						std::string mvalstr = mval->value_.get_str();
						outFilenames.push_back( mvalstr );
						continue;
					}
					if( mattr == "size" ) {
						const auto& sobj = mval->value_.get_obj();
						for( auto sval = sobj.cbegin(); sval != sobj.cend(); ++sval ) {
							std::string sattr = sval->name_;
							boost::algorithm::to_lower( sattr );
							if( sattr == "w" ) {
								texWidth = sval->value_.get_int();
								continue;
							}
							if( sattr == "h" ) {
								texHeight = sval->value_.get_int();
								continue;
							}
						}
					}
				}
				continue;
			}

			if( attr == "frames" ) {
				const auto& arr = val->value_.get_array();
				for( auto aval = arr.cbegin(); aval != arr.cend(); ++aval ) { 
					Export::SubTexture st;
					st.index = 0; // texpacker only supports one texture so far
					bool rotated = false;
					const auto& fobj = aval->get_obj();
					for( auto fval = fobj.cbegin(); fval != fobj.cend(); ++fval ) {
						std::string fattr = fval->name_;
						boost::algorithm::to_lower( fattr );
						if( fattr == "frame" ) {
							const auto& sobj = fval->value_.get_obj();
							for( auto sval = sobj.cbegin(); sval != sobj.cend(); ++sval ) {
								std::string sattr = sval->name_;
								boost::algorithm::to_lower( sattr );
								if( sattr == "x" ) {
									st.u0 = sval->value_.get_int();
									continue;
								}
								if( sattr == "y" ) {
									st.v0 = sval->value_.get_int();
									continue;
								}						
								if( sattr == "w" ) {
									st.u1 = sval->value_.get_int();
									continue;
								}
								if( sattr == "h" ) {
									st.v1 = sval->value_.get_int();
									continue;
								}
							}
							continue;
						}
						if( fattr == "rotated" ) {
							rotated = fval->value_.get_bool();
							continue;
						}
					}
					// we placed the w and h in u/v1 now to make them proper
					st.u1 += st.u0;
					st.v1 += st.v0;
					
					// now rotate if required
					if( rotated ) {
						std::swap( st.u0, st.v0 );
						std::swap( st.u1, st.v1 );
					}
					// now push but still not quite ready
					outSprites.push_back( st );
					continue;
				}
			}
		}
		// now we need to convert into normalised coords
		for( auto it = outSprites.begin(); it != outSprites.end(); ++it ) {
			(*it).u0 /= (float) texWidth;
			(*it).u1 /= (float) texWidth;
			(*it).v0 /= (float) texHeight;
			(*it).v1 /= (float) texHeight;
		}
	}
}