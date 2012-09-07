/**
 @file	Z:\Projects\wierd\source\gl\memory.h

 @brief	Declares the memory class.
 */
#pragma once
#if !defined( YOLK_YOLK_RESOURCE_H_ )
#define YOLK_YOLK_RESOURCE_H_

#include "scene/resource.h"

namespace Dx11 {
	class Dx11View : public Scene::View {
	public:
		Dx11View( D3DViewPtr _view ) : view(_view) {}

		D3DViewPtr view;
	};

	typedef std::shared_ptr<Dx11View> Dx11ViewPtr;

	class Resource {
	public:
		Resource( D3DResourcePtr _resource ) : resource(_resource) {}

		virtual ~Resource() {
			views.clear();
			resource.reset();
		}

		D3DResourcePtr get() const { return resource; }

		Dx11ViewPtr getDx11View( uint32_t viewType ) const { return views[viewType]; }

		// for custom view or in CUSTOM_VIEW for type and it will create the view and return its index
		uint32_t createView( uint32_t viewType, const Scene::Resource::CreationInfo* creation );
	protected:
		void createSRView( uint32_t index, const Scene::Resource::CreationInfo* creation );
		void createDSView( uint32_t index, const Scene::Resource::CreationInfo* creation );
		void createRTView( uint32_t index, const Scene::Resource::CreationInfo* creation );
		void createUAView( uint32_t index, const Scene::Resource::CreationInfo* creation );

		D3DResourcePtr			resource;
		std::vector<Dx11ViewPtr>	views;
	};
}

#endif