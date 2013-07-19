#pragma once
//!-----------------------------------------------------
//!
//! \file voxtreerenderable.h
//! renderable element composed of a single voxtree
//!
//!-----------------------------------------------------
#if !defined( YOLK_VOXTREERENDERABLE_H_ )
#define YOLK_VOXTREERENDERABLE_H_

#if !defined( YOLK_SCENE_RENDERABLE_H_ )
#include "scene/renderable.h"
#endif

#if !defined( YOLK_VOXTREE_H_ )
#include "voxtree.h"
#endif


namespace Vox {

class WallRenderer;

class TreeRenderable : public Scene::Renderable {
public:
	static const uint32_t VOXR_TYPE = Core::GenerateID<'V','O','X','R'>::value;

	TreeRenderable( Core::TransformNode* _transform, const Tree& _vtree );
	virtual ~TreeRenderable();

	//--------- RENDERABLE IMPLEMENTATION START -------------

	//! this is a method to get a flat array of renderables. For simple
	//! renderable it will 0 or 1 entry, however for compound renderables
	//! it can render a number of renderables. Its also allows easy filtering
	//! by type. Will return upto _maxArraySize actual renderables of _type in _outArray,
	//! \param _type 4 char code to tell what type to return (ALL_TYPES for all)
	//! \param _maxArraySize size of the array passed in, so also max that can be returned
	//! \param _outArray pointer to an array of pointer of size _maxArraySize to be filled
	//! \return number in array
	void getRenderablesOfType( uint32_t _type, std::vector<Renderable*>& _out ) const override;
	void getVisibleRenderablesOfType( const Core::Frustum& _frustum, const uint32_t _type, std::vector< Renderable*>& _out ) const override;

	//--------- RENDERABLE IMPLEMENTATION END -------------

protected:
	const Tree& 						tree;
	mutable WallRenderer*		wallRenderer;

};
typedef std::shared_ptr<TreeRenderable>						TreeRenderablePtr;
	
} // end Vox namespace

#endif // end YOLK_VOXTREERENDERABLE_H_