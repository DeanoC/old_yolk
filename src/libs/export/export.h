#include "core/core.h"
#include "core/coreresources.h"
#include "core/file_path.h"
#include <iostream>
#include <fstream>

#include "meshmod/scene.h"
#include "meshmod/sceneobject.h"
#include "meshmod/mesh.h"
#include "meshmod/vertexdata.h"
#include "meshmod/uvvertex.h"

namespace Export {
	extern bool SaveManifest( const Core::ResourceManifestEntryVector& manifest,  const Core::FilePath pOutFilename );

	extern bool SaveHierachy( MeshMod::ScenePtr scene, Core::ResourceManifestEntryVector& manifest, 
                               													  const Core::FilePath pOutFilename );

	extern bool SaveMeshes( MeshMod::SceneNodePtr root, Core::ResourceManifestEntryVector& manifest, 
                               													  const Core::FilePath pOutFilename );
	extern bool SaveProps( MeshMod::ScenePtr scene, Core::ResourceManifestEntryVector& manifest, const Core::FilePath pOutFilename );

}

extern void Binify( const std::string& txt, std::ostream& out );
