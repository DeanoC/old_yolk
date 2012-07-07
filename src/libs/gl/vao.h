/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( WIERD_GL_VAO_H__ )
#define WIERD_GL_VAO_H__

#include "core/resources.h"
#include "memory.h"
#include "databuffer.h"

namespace Gl {
	static const uint32_t VaoRType = RESOURCE_NAME('G','V','A','O');

	//! what this vertex type is for
	enum VAO_ELEMENT {
		VE_POSITION = 0,
		VE_NORMAL,
		VE_BINORMAL,
		VE_TANGENT,
		VE_TEXCOORD0,
		VE_TEXCOORD1,
		VE_TEXCOORD2,
		VE_TEXCOORD3,
		VE_COLOUR0,
		VE_COLOUR1,
		VE_BONEINDICES,
		VE_BONEWEIGHTS,
	};

	//! what the data type is
	enum VAO_TYPE {
		VT_FLOAT3 = 0,
		VT_FLOAT2,
		VT_BYTEARGB,
		VT_FLOAT1,
		VT_FLOAT4,
		VT_SHORT2
	};

	//! defines a vertex element, consists of a usage and a format
	struct VaoElement {
		uint16_t usage;	//!< what the usage is
		uint16_t type;	//!< what the type is

		//! compares to vertex element arrays for equality
		static bool Compare( int numElements, const VaoElement *lhs, const VaoElement* rhs ) {
			for( int i=0; i < numElements; ++i ) {
				if( lhs->usage != rhs->usage || lhs->type != rhs->type )
					return false;
				++lhs; ++rhs;
			}
			return true;
		}
	};

	class Vao :	public Memory,
				public Core::Resource<VaoRType> {
	public:
		static const int MAX_ELEMENT_COUNT = 16;
		static const size_t AUTO_OFFSET = ~0;
		static const uint32_t AUTO_STRIDE = 0xFFFFFF;

		struct CreationStruct {
			int						elementCount;			//<! how many vertex elements
			DataBufferHandlePtr		indexBuffer;			//!< indexBuffer, null if not
			VaoElement				elements[MAX_ELEMENT_COUNT];
			struct {
				DataBufferHandlePtr		buffer;
				size_t					offset;				//!< AUTO_OFFSET == packed
				uint32_t				stride : 24;		//!< AUTO_STRIDE == packed
				uint32_t				stream : 8;			//!< stream number
			} data[MAX_ELEMENT_COUNT];
		};
		struct LoadStruct {};
		static Vao* internalCreate(	const Core::ResourceHandleBase* baseHandle, 
									const char* pName, const CreationStruct* creation );

		static uint32_t getElementItemCount( const VaoElement element );
		static GLenum getElementGlType( const VaoElement element );
		static size_t getElementSize( const VaoElement element );
		static size_t getVertexSize( int numElements, const VaoElement *type );
		static std::string genEleString( int numElements, const VaoElement *type );

	protected:
		Vao();
	};

	typedef const Core::ResourceHandle<VaoRType, Vao> VaoHandle;
	typedef VaoHandle* VaoHandlePtr;
	typedef std::shared_ptr<Vao> VaoPtr;

}

#endif