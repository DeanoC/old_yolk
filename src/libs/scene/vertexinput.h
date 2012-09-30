/**
 @file	Z:\Projects\wierd\source\gl\databuffer.h

 @brief	Declares the databuffer class.
 */
#pragma once
#if !defined( YOLK_SCENE_VERTEXINPUT_H_ )
#define YOLK_SCENE_VERTEXINPUT_H_

#if !defined( YOLK_CORE_RESOURCES_H_ )
#	include "core/resources.h"
#endif
#if !defined( YOLK_CORE_DATABUFFER_H_ )
#	include "databuffer.h"
#endif
#if !defined( YOLK_CORE_PROGRAM_H_ )
#	include "program.h"
#endif

namespace Scene {
	static const uint32_t VertexInputType = RESOURCE_NAME('G','V','I','N');

	//! what this vertex type is for
	enum VIN_ELEMENT {
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
	enum VIN_TYPE {
		VT_FLOAT3 = 0,
		VT_FLOAT2,
		VT_BYTEARGB,
		VT_FLOAT1,
		VT_FLOAT4,
		VT_SHORT2
	};

	//! defines a vertex element, consists of a usage and a format
	struct VinElement {
		uint16_t usage;	//!< what the usage is
		uint16_t type;	//!< what the type is

		//! compares to vertex element arrays for equality
		static bool Compare( int numElements, const VinElement *lhs, const VinElement* rhs ) {
			for( int i=0; i < numElements; ++i ) {
				if( lhs->usage != rhs->usage || lhs->type != rhs->type )
					return false;
				++lhs; ++rhs;
			}
			return true;
		}
	};
	static const size_t VI_AUTO_OFFSET = ~0;
	static const uint32_t VI_AUTO_STRIDE = 0xFFFF;
	static const uint32_t VI_VERTEX_STREAM = 0;

	class RenderContext;

	class VertexInput : public Core::Resource<VertexInputType> {
	public:
		friend class ResourceLoader;
		virtual ~VertexInput(){}

		static const int MAX_ELEMENT_COUNT = 16;

		struct CreationInfo {
			int								elementCount;			//<! how many vertex elements
			VinElement						elements[MAX_ELEMENT_COUNT];
			struct {
				Scene::DataBufferHandlePtr		buffer;
				size_t							offset;					//!< AUTO_OFFSET == packed
				uint32_t						stride : 16;			//!< AUTO_STRIDE == packed
				uint32_t						stream : 4;				//!< stream number
				uint32_t						instanceCount : 12;		//!< 0 = vertex else number if instances to pass for each advance this stream
			} data[MAX_ELEMENT_COUNT];
		};

		static uint32_t getElementItemCount( const VinElement& element );
		static size_t getElementSize( const VinElement& element );
		static size_t getVertexSize( int numElements, const VinElement *type );
		static std::string genEleString( int numElements, const VinElement *type );

		// each vertex input must be validated once with a shader that uses it
		virtual void validate( Scene::ProgramPtr program ) = 0;
		bool isValid() { return valid; }

	protected:
		bool valid;
		VertexInput() : valid( false ) {};
		static const void* internalPreCreate( const char* name, const CreationInfo *loader ) { return loader; };
	};

	typedef const Core::ResourceHandle<VertexInputType, VertexInput> VertexInputHandle;
	typedef VertexInputHandle* VertexInputHandlePtr;
	typedef std::shared_ptr<VertexInput> VertexInputPtr;

}

#endif