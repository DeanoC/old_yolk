#include "core/core.h"
#include "core/coreresources.h"
#include "core/fileio.h"
#include "core/stb_image.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include "meshimport/meshimport.h"
//#include "meshimport/assimp.h"
#include "meshimport/grometxtimp.h"
#include "meshimport/LWObject2GO.h"
#include "meshimport/LWScene2GO.h"
#include "export/export.h"
#include "export/wobexp.h"
#include "tof.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
void OutputScene( Core::FilePath outPath, std::shared_ptr<MeshImport::ImportInterface> importer );
void DoTexture( const Core::FilePath& inPath, const Core::FilePath& outPath );
void DoTextureAtlas( const Core::FilePath& inPath, const Core::FilePath& outPath );

//--------------------------------------------------------------------------------------
// Entry point to the program.
//--------------------------------------------------------------------------------------
int Main() {
	using namespace Core;

	// Declare the supported options.
	namespace po = boost::program_options;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show the usage of the program")
		("input", po::value<std::string>(), "Mesh/texture/texture atlas to be converted (obj, lwo, dae, png, tao + more)")
		("output", po::value<std::string>(), "filename of output name" );
	po::positional_options_description posi;
	posi.add("input", 1);
	posi.add("output", 1);

	po::parsed_options parsed = 
		po::command_line_parser(g_argc, g_argv).
			options(desc).
			positional(posi).
			allow_unregistered().run(); 
	po::variables_map vm;        
	po::store( parsed, vm);
	po::notify(vm);    

	if ( vm.count("help") || g_argc < 2 || !vm.count( "input" ) ) {
		LOG(INFO) << desc << "\n";
		return 1;
	}

	const std::string cwd = boost::filesystem::current_path().string();

	const std::string inname = vm["input"].as<std::string>();
	FilePath inPath( inname );
	FilePath outPath;
	// if no output specified use the input filename in the local directory 
	if( g_argc == 2 ) {
		outPath = FilePath(cwd).Append( inPath.BaseName() );
	} else {
		outPath = FilePath( vm["output"].as<std::string>() );
		if( !outPath.IsAbsolute() && !FilePath::IsSeparator( outPath.value()[0] ) ) {
			outPath = FilePath(cwd).Append( outPath );
		} 
	}

	using namespace MeshImport;
	using namespace MeshMod;
	using namespace Export;

	auto ext = inPath.Extension();

	if(  ext == ".lws" || ext == ".lwo" ) {
		LOG(INFO) << "Input Path : " << inPath.DirName().value().c_str() << "\n";
		boost::filesystem::current_path( inPath.DirName().value().c_str() );

		std::shared_ptr<ImportInterface> importer;
		if( inPath.Extension() == ".lwo" ) {
			importer = std::make_shared<LWOImp>( inPath.BaseName().value() );
		} else {
			importer = std::make_shared<LWSImp>( inPath.BaseName().value() );
		}
		if( importer->loadedOk() == false ) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene( outPath, importer );
	} else if( ext == ".txt" ) {
		LOG(INFO) << "Input Path : " << inPath.DirName().value().c_str() << "\n";
		boost::filesystem::current_path( inPath.DirName().value().c_str() );

		std::shared_ptr<ImportInterface> importer;
		importer = std::make_shared<GromeTxtImp>( inPath.BaseName().value() );
		if( importer->loadedOk() == false ) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene( outPath, importer );
	} else
	if(	ext == ".png" || ext == ".tga" ||
		ext == ".jpg" || ext == ".psd" ||
		ext == ".bmp" || ext == ".gif" ) {

		DoTexture( inPath, outPath );

	} else
	if( ext == ".hdr" ) {
	} else
	if( ext == ".tao" ) {
		DoTextureAtlas( inPath, outPath );
	} else
	{
#if 0
	std::shared_ptr<ImportInterface> importer;
		importer = std::make_shared<AssImp>( inPath.BaseName().value() );
		if( importer->loadedOk() == false ) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene( outPath, importer );
#endif
		LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> can not be covnerted\n";

	}

	return 0;
}

void OutputScene( Core::FilePath outPath, std::shared_ptr<MeshImport::ImportInterface> importer ) {
	using namespace Core;
	using namespace MeshImport;
	using namespace MeshMod;
	using namespace Export;

	LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path( outPath.DirName().value().c_str() );

	Core::ResourceManifestEntryVector manifest;
	auto outBaseName = outPath.BaseName();

	ScenePtr scene = importer->toMeshMod();
	for( auto nodeIt = scene->sceneNodes.begin(); nodeIt != scene->sceneNodes.end(); ++nodeIt ) {
		auto rootNode = *nodeIt;

		// save out the meshes and models themselves
		SaveMeshes( rootNode, manifest, outBaseName );
	}
	
	// save out the scene representation
	SaveHierachy( scene, manifest, outBaseName );
	// and the properties
	SaveProps( scene, manifest, outBaseName );

	// write out a manifest of the files for preloading/caching
	SaveManifest( manifest, outBaseName );
}

void DoTexture( const Core::FilePath& inFullPath, const Core::FilePath& outPath ) {
	LOG(INFO) << "Input Path : " << inFullPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path( inFullPath.DirName().value().c_str() );

	Core::FilePath inPath = inFullPath.BaseName();

	using namespace Export;
	Tof tof = loadTof( inPath );

	TextureExport tex;

	// load the bitmaps 
	for( auto fileIt = tof.files.cbegin(); 
		 fileIt != tof.files.cend();
		 ++fileIt ) {

		Core::MemFile fio( fileIt->c_str() );
		if( !fio.isValid() ) {
			LOG(INFO) << *fileIt << " not found\n";
			continue;
		}

		stbi_io_callbacks ioCallbacks;
		ioCallbacks.read = &Core::InOutInterface::C_read;
		ioCallbacks.skip = &Core::InOutInterface::C_skip;
		ioCallbacks.eof = &Core::InOutInterface::C_eof;
		int width = 0, height = 0, components = 0;
		uint8_t* data = stbi_load_from_callbacks( 
						&ioCallbacks, 
						(void*)&fio.inOut(), 
						&width, 
						&height, 
						&components, 0 );

		BitmapInput bi;
		bi.flags = 0; // TODO hdr
		bi.width = width;
		bi.height = height;
		bi.data = data;
		switch( components ) {
			case STBI_grey: bi.channels = 1; break;
			case STBI_grey_alpha: bi.channels = 2; break;
			case STBI_rgb: bi.channels = 3; break;
			case STBI_rgb_alpha: 
				bi.channels = 4; 
				bi.flags |= BitmapInput::BI_RGBA; 
				break;
			default:
				LOG(INFO) << "Invalid component count for " << *fileIt << "\n";
				bi.channels = 0;
				break;
		}
		if( bi.channels > 0 ) {
			tex.bitmaps.push_back( bi );
		}
	}

	if( !tex.bitmaps.empty() ) {

		tex.outFormat = (GENERIC_TEXTURE_FORMAT) tof.format;
		tex.outFlags = 0;
		tex.outWidth = tof.width;
		tex.outHeight = tof.height;
		tex.outDepth = tof.depth;
		tex.outSlices = tof.arraySize;
		tex.outMipMapCount = 1; // TODO

		// handle defaults, use first bitmap
		if( tof.format == TOF_DEFAULT ) {
			uint32_t channels = tex.bitmaps[0].channels;
			switch( channels ) {
				case 1: tex.outFormat = GTF_R8; break;
				case 2: tex.outFormat = GTF_RG8; break;
				case 3:
				default:
					if( tof.linear == false ) {
						tex.outFormat = GTF_SRGB8; 
					} else {
						tex.outFormat = GTF_RGB8; 
					}
					break;
				case 4: 					
					if( tof.linear == false ) {
						tex.outFormat = GTF_SRGB8_ALPHA8; 
					} else {
						tex.outFormat = GTF_RGBA8; 
					}
					break;
			}
		}
		if( tof.width == TOF_DEFAULT ) {
			tex.outWidth = tex.bitmaps[0].width;
		}
		if( tof.height == TOF_DEFAULT ) {
			tex.outHeight = tex.bitmaps[0].height;
		}
		if( tof.depth == TOF_DEFAULT && !tof.cubeMap) {
			tex.outDepth = tex.bitmaps.size();
		}
		if( tof.depth != TOF_DEFAULT && tof.arraySize == TOF_DEFAULT ) {
			tex.outSlices = tex.bitmaps.size() / tof.depth;
		} else {
			tex.outSlices = 1;
		}
		if( tof.cubeMap ) {
			tex.outFlags |= TextureExport::TE_CUBEMAP;
			if( (tex.bitmaps.size() % 6) != 0 ) {
				LOG(INFO) << "Cubemap (array) missing faces for " << inPath.value() << "\n";
			}
			tex.outSlices = tex.bitmaps.size(); // faces are in the arrays
			tex.outDepth = 0; // depth cubemaps make no sense
		}

		LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
		boost::filesystem::current_path( outPath.DirName().value().c_str() );

		Export::SaveTexture( tex, outPath.BaseName() );

	} else {
		LOG(INFO) << "No bitmaps are valid for " << inPath.value() << "\n";
	}

}

void DoTextureAtlas( const Core::FilePath& inFullPath, const Core::FilePath& outPath ) {
	LOG(INFO) << "Input Path : " << inFullPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path( inFullPath.DirName().value().c_str() );

	Core::FilePath inPath = inFullPath.BaseName();

	using namespace Export;
	std::vector<std::string> filenames;
	std::vector<SubTexture> sprites;

	loadTao( inPath, filenames, sprites );

	for( 	auto fiIt = filenames.cbegin(); 
			fiIt != filenames.cend(); 
			++fiIt ) {
		DoTexture( Core::FilePath(*fiIt), outPath );
	}

	LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path( outPath.DirName().value().c_str() );

	SaveTextureAtlas( filenames, sprites, outPath );
}
