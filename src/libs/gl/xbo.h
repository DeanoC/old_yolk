/**
 @file	Z:\Projects\wierd\source\gl\xbo.h

 @brief	Declares the transform buffer object class.
 */
#pragma once
#if !defined( WIERD_GL_XBO_H__ )
#define WIERD_GL_XBO_H__

#include "core/resources.h"
#include "memory.h"
#include "databuffer.h"

namespace Gl {
	static const uint32_t XboRType = RESOURCE_NAME('G','X','B','O');

	class Xbo :	public Memory,
				public Core::Resource<XboRType> {
	public:

		struct CreationStruct {};
		struct LoadStruct {};

		static Xbo* internalCreate(	const Core::ResourceHandleBase* baseHandle, const char* pName ) {
			Xbo* xbo = CORE_NEW Xbo();
			xbo->generateName( MNT_TRANSFORM_FEEDBACK_BUFFER );
			return xbo;
		}

		void setBuffer( int index, const DataBufferHandlePtr& buffer ) {
			auto buf = buffer->acquire();
			glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, getName() ); 
			glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, index, buf->getName() );
			glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 ); 
		}

	protected:
		Xbo(){};
	};

	typedef const Core::ResourceHandle<XboRType, Xbo> XboHandle;
	typedef XboHandle* XboHandlePtr;
	typedef std::shared_ptr<Xbo> XboPtr;

}

#endif