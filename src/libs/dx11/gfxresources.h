//!-----------------------------------------------------
//!
//! \file gfxresources.h
//! holds all the graphic resources stuff etc.
//!
//!-----------------------------------------------------
#if !defined( YOLK_DX11_GFXRESOURCES_H_ )
#define YOLK_DX11_GFXRESOURCES_H_
#pragma once

#if !defined( YOLK_CORE_RESOURCES_H_ )
#include "core/resources.h"
#endif

#if !defined( YOLK_DX11_WOBFILE_H_ )
#include "wobfile.h"
#endif

namespace Core {
	class BVH;
};
namespace Dx11 {	

	//! An actual wob resource, has a wob file headers and the materials
	//! the file header is patched to point directly the the materials here
	struct WobResource : public Core::Resource<WobType> {
		struct CreationStruct {};
		struct LoadStruct {};

		Core::shared_ptr<WobFileHeader>		m_spHeader;
		Core::scoped_array<class MaterialWob>	m_saMaterials;

		Core::scoped_ptr<Core::BVH>			m_bvh;

		typedef void (*PreDiscardLoadCallback)( WobResource* res );
		static void RegisterPreDiscardLoadCallback( PreDiscardLoadCallback func ) {
			s_loadPreDiscardCallback = func;
		}
		static PreDiscardLoadCallback s_loadPreDiscardCallback;
	};

	typedef Core::shared_ptr<WobResource> WobResourcePtr;
	//! A wob file resource handle typedef
	typedef const Core::AsyncResourceHandle<WobType, WobResource> WobResourceHandle;
	typedef WobResourceHandle* WobResourceHandlePtr;

	struct WobHierachyResource : public Core::Resource<WobHierType> {
		struct CreationStruct {};
		struct LoadStruct {};
		Core::shared_ptr<WobHierarchyFileHeader>	m_spHeader;
	};
	typedef Core::shared_ptr<WobHierachyResource> WobHierachyResourcePtr;
	//! A wob file resource handle typedef
	typedef const Core::AsyncResourceHandle<WobHierType, WobHierachyResource> WobHierachyResourceHandle;
	typedef WobHierachyResourceHandle* WobHierachyResourceHandlePtr;

} // end namespace Graphics

#endif //WIERD_GRAPHICS_GFXRESOURCES_H
