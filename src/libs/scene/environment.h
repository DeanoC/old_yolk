#ifndef YOLK_SCENE_ENVIRONMENT_H_
#define YOLK_SCENE_ENVIRONMENT_H_ 1
#pragma once

#include "core/coreresources.h"

namespace Scene {

	class Environment {
	public:
		bool hasGravity() const { return Math::LengthSquared(getGravity()) > 1e-5f; }
		Math::Vector3 getGravity() const;

		Environment( Core::BinPropertyResourceHandlePtr handle );
		~Environment();
		
		Core::BinPropertyResourceHandlePtr getPropertiesResourceHandle() const {
			return properties.get()->clone();
		}

	protected:
		Core::ScopedResourceHandle< Core::BinPropertyResourceHandle > properties;
		Core::BinPropertyResourcePtr res;
		const float* gravityPtr;
	};

	typedef std::shared_ptr<Environment>						EnvironmentPtr;

}

#endif