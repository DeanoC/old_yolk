#pragma once
#ifndef YOLK_SCENE_DYNAMIC_PHYSICAL
#define YOLK_SCENE_DYNAMIC_PHYSICAL 1

#include "physical.h"
namespace Scene {
	class DynamicPhysical : public Physical {
	public:

		explicit DynamicPhysical( 	Core::TransformNode* node,
									std::shared_ptr<CollisionShape> _shape, 
									float _mass = 1.0f, 
									Math::Vector3 _localInertia = Math::Vector3(0,0,0),
									float _friction = 0.5f,
									float _restitution = 0.f,
									float _linearDamping = 0.f,
									float _angularDamping = 0.f );

		virtual float getMass() const override { return mass; }
		virtual Math::Vector3 getLocalInertia() const override{ return localInertia; }

		void setMassProps( const float _mass, const Math::Vector3& _localInertia );

	protected:
		float					mass;
		Math::Vector3			localInertia;

	};
}
#endif