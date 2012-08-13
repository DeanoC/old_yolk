#pragma once
#if !defined( YOLK_GL_SCREEN_H_ )
#define YOLK_GL_SCREEN_H_

#include "scene/screen.h"

namespace Gl {

	class Screen : public Scene::Screen {
	public:
		friend class Gfx;
	};
}
#endif