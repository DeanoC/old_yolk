#if !defined( YOLK_MESHMOD_MESHIMPORT_H_ )
#define YOLK_MESHMOD_MESHIMPORT_H_

#include <core/core.h>

#include <meshmod/scene.h>
#include <meshmod/sceneobject.h>
#include <meshmod/mesh.h>
#include <meshmod/vertexdata.h>
#include <meshmod/uvvertex.h>

namespace MeshImport {
	class ImportInterface {
	public:
		virtual ~ImportInterface(){};
		virtual bool loadedOk() const = 0;
		virtual MeshMod::ScenePtr toMeshMod() = 0;
	};
}

#endif