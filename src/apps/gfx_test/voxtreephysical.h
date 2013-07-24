#pragma once
#ifndef YOLK_VOXTREEPHYSICAL_H_
#define YOLK_VOXTREEPHYSICAL_H_ 1

#include "scene/physical.h"
namespace Scene {
	class BoxColShape;
	class CompoundColShape;
};

namespace Vox {
	template< typename TreeType >
	class TreePhysical : public Scene::Physical {
	public:
		typedef std::shared_ptr<TreePhysical<TreeType>> Ptr;

		TreePhysical( Core::TransformNode* _transform, const TreeType& tree );
	protected:
		std::shared_ptr<Scene::BoxColShape> getCubeColShape( const Core::AABB _aabb );

		const TreeType& tree;
		std::unordered_map< float, std::shared_ptr<Scene::BoxColShape> > cubeShapes;
		std::shared_ptr<Scene::CompoundColShape>	compoundShape;
	};

};

#include "voxtreephysical.inl"

#endif