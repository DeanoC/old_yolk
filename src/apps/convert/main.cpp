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
#include "do.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
void OutputScene(Core::FilePath outPath, std::shared_ptr<MeshImport::ImportInterface> importer);

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
		("output", po::value<std::string>(), "filename of output name");
	po::positional_options_description posi;
	posi.add("input", 1);
	posi.add("output", 1);

	po::parsed_options parsed =
		po::command_line_parser(g_argc, g_argv).
		options(desc).
		positional(posi).
		allow_unregistered().run();
	po::variables_map vm;
	po::store(parsed, vm);
	po::notify(vm);

	if (vm.count("help") || g_argc < 2 || !vm.count("input")) {
		LOG(INFO) << desc << "\n";
		return 1;
	}

	const std::string cwd = boost::filesystem::current_path().string();

	const std::string inname = vm["input"].as<std::string>();
	FilePath inPath(inname);
	FilePath outPath;
	// if no output specified use the input filename in the local directory 
	if (g_argc == 2) {
		outPath = FilePath(cwd).Append(inPath.BaseName());
	}
	else {
		outPath = FilePath(vm["output"].as<std::string>());
		if (!outPath.IsAbsolute() && !FilePath::IsSeparator(outPath.value()[0])) {
			outPath = FilePath(cwd).Append(outPath);
		}
	}

	using namespace MeshImport;
	using namespace MeshMod;
	using namespace Export;

	auto ext = inPath.Extension();

	if (ext == ".lws" || ext == ".lwo") {
		LOG(INFO) << "Input Path : " << inPath.DirName().value().c_str() << "\n";
		boost::filesystem::current_path(inPath.DirName().value().c_str());

		std::shared_ptr<ImportInterface> importer;
		if (inPath.Extension() == ".lwo") {
			importer = std::make_shared<LWOImp>(inPath.BaseName().value());
		}
		else {
			importer = std::make_shared<LWSImp>(inPath.BaseName().value());
		}
		if (importer->loadedOk() == false) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() << "> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene(outPath, importer);
	}
	else if (ext == ".txt") {
		LOG(INFO) << "Input Path : " << inPath.DirName().value().c_str() << "\n";
		boost::filesystem::current_path(inPath.DirName().value().c_str());

		std::shared_ptr<ImportInterface> importer;
		importer = std::make_shared<GromeTxtImp>(inPath.BaseName().value());
		if (importer->loadedOk() == false) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() << "> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene(outPath, importer);
	}
	else if (ext == ".png" || ext == ".tga" ||
		ext == ".jpg" || ext == ".psd" ||
		ext == ".bmp" || ext == ".gif") {

		DoTexture(inPath, outPath);

	}
	else if (ext == ".hdr") {
	}
	else if (ext == ".tao") {
		DoTextureAtlas(inPath, outPath);
	}
	else if (ext == ".ttf") {
		DoTrueTypeFont(inPath, outPath, PACK_BEFORE_DISTANCE);
	}
	else {
#if 0
		std::shared_ptr<ImportInterface> importer;
		importer = std::make_shared<AssImp>( inPath.BaseName().value() );
		if( importer->loadedOk() == false ) {
			LOG(INFO) << "Input file <" << inPath.BaseName().value() <<"> not found or unable to be loaded\n";
			return 1;
		}
		OutputScene( outPath, importer );
#endif
		LOG(INFO) << "Input file <" << inPath.BaseName().value() << "> can not be covnerted\n";

	}

	return 0;
}

void OutputScene(Core::FilePath outPath, std::shared_ptr<MeshImport::ImportInterface> importer) {
	using namespace Core;
	using namespace MeshImport;
	using namespace MeshMod;
	using namespace Export;

	LOG(INFO) << "Output Path : " << outPath.DirName().value().c_str() << "\n";
	boost::filesystem::current_path(outPath.DirName().value().c_str());

	Core::ResourceManifestEntryVector manifest;
	auto outBaseName = outPath.BaseName();

	ScenePtr scene = importer->toMeshMod();
	for (auto nodeIt = scene->sceneNodes.begin(); nodeIt != scene->sceneNodes.end(); ++nodeIt) {
		auto rootNode = *nodeIt;

		// save out the meshes and models themselves
		SaveMeshes(rootNode, manifest, outBaseName);
	}

	// save out the scene representation
	SaveHierachy(scene, manifest, outBaseName);
	// and the properties
	SaveProps(scene, manifest, outBaseName);

	// write out a manifest of the files for preloading/caching
	SaveManifest(manifest, outBaseName);
}

