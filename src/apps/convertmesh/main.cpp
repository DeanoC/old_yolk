#include "core/core.h"
#include "core/coreresources.h"
#include <boost/program_options.hpp>
#include "meshimport/meshimport.h"
#include "meshimport/assimp.h"
#include "meshimport/grometxtimp.h"
#include "meshimport/LWScene2GO.h"
#include "export/export.h"
#include "export/wobexp.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Entry point to the program.
//--------------------------------------------------------------------------------------
int Main() {
	namespace po = boost::program_options;
	using namespace Core;

	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "Show the usage of the program")
		("input", po::value<std::string>(), "Mesh to be converted (obj,lwo, dae, more)")
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

	char cwd[ PATH_MAX ];
	if( getcwd( cwd, PATH_MAX ) == NULL ) {
		LOG(INFO) << "Directory failure\n";
		return 1;
	}

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

	LOG(INFO) << "Input Path : " << inPath.DirName().value().c_str() << "\n";
	chdir( inPath.DirName().value().c_str() );

	std::shared_ptr<ImportInterface> importer;

	SceneNodePtr rootNode;
	if( inPath.Extension() == ".lws" ) {
		importer = std::make_shared<LWSImp>( inPath.BaseName().value() );
	} else 
	if( inPath.Extension() == ".txt" ) {
		importer = std::make_shared<GromeTxtImp>( inPath.BaseName().value() );
	} else 
	{
		importer = std::make_shared<AssImp>( inPath.BaseName().value() );
	}
	
	if( importer->loadedOk() == false ) {
		LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> not found or unable to be loaded\n";
		return 1;
	}

	LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
	chdir( outPath.DirName().value() .c_str() );

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

	return 0;
}
