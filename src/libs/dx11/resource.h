/**
 @file	Z:\Projects\wierd\source\gl\memory.h

 @brief	Declares the memory class.
 */
#pragma once
#if !defined( YOLK_YOLK_RESOURCE_H_ )
#define YOLK_YOLK_RESOURCE_H_

#include "scene/resource.h"

namespace Dx11 {

	class Resource {
	public:
		enum VIEW_TYPE {
			// the first slots are hardcoded to views created at resource creation time (depending on what was created)
			SHADER_RESOURCE_VIEW = 0,
			DEPTH_STENCIL_VIEW,
			RENDER_TARGET_VIEW,
			UNORDERED_ACCESS_VIEW,
			CUSTOM_VIEW 		= BIT(31),
		};

		Resource( D3DResourcePtr _resource ) : resource(_resource) {}

		virtual ~Resource() {
			views.clear();
			resource.reset();
		}

		D3DResourcePtr get() const { return resource; }

		D3DViewPtr getView( uint32_t viewType ) const { return views[viewType]; }

		// for custom view or in CUSTOM_VIEW for type and it will create the view and return its index
		uint32_t createView( uint32_t viewType, Scene::Resource::CreationInfo* creation );
	protected:
		void createSRView( uint32_t index, Scene::Resource::CreationInfo* creation );
		void createDSView( uint32_t index, Scene::Resource::CreationInfo* creation );
		void createRTView( uint32_t index, Scene::Resource::CreationInfo* creation );
		void createUAView( uint32_t index, Scene::Resource::CreationInfo* creation );

		D3DResourcePtr			resource;
		std::vector<D3DViewPtr>	views;
	};
}

#endif