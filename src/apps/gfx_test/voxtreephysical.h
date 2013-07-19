#pragma once
#ifndef YOLK_VOXTREEPHYSICAL_H_
#define YOLK_VOXTREEPHYSICAL_H_ 1

#include "scene/physical.h"
namespace Scene {
	class BoxColShape;
	class CompoundColShape;
};

namespace Vox {
	class TreePhysical : public Scene::Physical {
	public:
		TreePhysical( Core::TransformNode* _transform, const Tree& tree );
	protected:
		std::shared_ptr<Scene::BoxColShape> getCubeColShape( const Core::AABB _aabb );

		const Tree& tree;
		std::unordered_map< float, std::shared_ptr<Scene::BoxColShape> > cubeShapes;
		std::shared_ptr<Scene::CompoundColShape>	compoundShape;
	};

	typedef std::shared_ptr<TreePhysical> TreePhysicalPtr;
};

#endif