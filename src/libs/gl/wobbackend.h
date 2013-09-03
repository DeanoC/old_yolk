#pragma once
///-------------------------------------------------------------------------------------------------++
/// \file	gl\wobbackend.h
///
/// \brief	Declares the Gl specific parts of a Wob.
///
/// \remark	Copyright (c) 2011 Dean Calver. All rights reserved.
/// \remark	mailto://deano@cloudpixies.com
///
////////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined( WIERD_GL_WOB_BACKEND_H__ )
#define WIERD_GL_WOB_BACKEND_H__

#include "scene/pipeline.h"
#include "scene/wobfile.h"

namespace Gl {
	class WobBackEnd : public Scene::WobBackEnd {
	public:
		WobBackEnd( int numPipelines ) {
			pipelineDataStores.resize( numPipelines );
		}
	};
}

#endif // WIERD_GL_WOB_BACKEND_H__
