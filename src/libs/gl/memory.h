/**
 @file	Z:\Projects\wierd\source\gl\memory.h

 @brief	Declares the memory class.
 */
#pragma once
#if !defined( WIERD_GL_MEMORY_H__ )
#define WIERD_GL_MEMORY_H__

namespace Gl {

	enum MEM_NAME_TYPE {
		MNT_FRAME_BUFFER = GL_FRAMEBUFFER,
		MNT_RENDER_BUFFER = GL_RENDERBUFFER,
		MNT_TRANSFORM_FEEDBACK_BUFFER = GL_TRANSFORM_FEEDBACK,
		MNT_DATA_BUFFER = GL_DATA_BUFFER_AMD,
		MNT_PERFORMANCE_MONITOR_OBJECT = GL_PERFORMANCE_MONITOR_AMD,
		MNT_TEXTURE_OBJECT = GL_TEXTURE,
		MNT_QUERY_OBJECT = GL_QUERY_OBJECT_AMD,
		MNT_VERTEX_ARRAY_OBJECT = GL_VERTEX_ARRAY_OBJECT_AMD,
		MNT_SAMPLER_OBJECT = GL_SAMPLER_OBJECT_AMD,
		MNT_VERTEX_SHADER_OBJECT = GL_VERTEX_SHADER,
		MNT_FRAGMENT_SHADER_OBJECT = GL_FRAGMENT_SHADER,
		MNT_GEOMETRY_SHADER_OBJECT = GL_GEOMETRY_SHADER,
		MNT_TESS_CONTROL_OBJECT = GL_TESS_CONTROL_SHADER,
		MNT_TESS_EVAL_SHADER_OBJECT = GL_TESS_EVALUATION_SHADER,
		MNT_PROGRAM_PIPELINE_OBJECT = GL_PROGRAM_PIPELINE_BINDING,
		MNT_PROGRAM_OBJECT = GL_PROGRAM_OBJECT_ARB,
	};

	class Memory {
	public:
		friend class ShaderMan; // sets name and type special like
		typedef GLuint Name;

		Memory() : name( 0 ) {}

		virtual ~Memory() {
			if( name != 0 ) {
				deleteName();
			}
		}

		const Name getName() const { return name; }
		const MEM_NAME_TYPE getType() const { return type; }

	protected:
		Name	name;
		MEM_NAME_TYPE type;
		// everything in opengl has a name
		static void generateNames( MEM_NAME_TYPE type, unsigned int num, Name* outNames );
		static void deleteNames( MEM_NAME_TYPE type, unsigned int num, Name* inNames );

		// internal helpers to name this memory object
		Name generateName( MEM_NAME_TYPE _type ) {
			type = _type;
			generateNames( type, 1, &name );
			return name;
		}

		void deleteName() {
			deleteNames( type, 1, &name );
			name = 0;
		}
	};
}

#endif