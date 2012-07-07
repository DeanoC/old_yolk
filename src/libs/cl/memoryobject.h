//!-----------------------------------------------------
//!
//! \file memoryobject.h
//! Contains the cl base class for compute memory
//!
//!-----------------------------------------------------
#if !defined(WIERD_CL_MEMORYOBJECT_H)
#define WIERD_CL_MEMORYOBJECT_H

namespace Cl {
	enum MEMOBJ_NAME_TYPE {
		MNT_BUFFER,
		MNT_SUB_BUFFER,
		MNT_IMAGE,
		MNT_GL_BUFFER,
		MNT_GL_TEXTURE,
	};

	class MemoryObject {
	public:
		typedef cl_mem Name;

		MemoryObject() : name( 0 ) {}

		virtual ~MemoryObject() {
			if( name != 0 ) {
				deleteName();
			}
		}

		const Name getName() const { return name; }
		const MEMOBJ_NAME_TYPE getType() const { return type; }

	protected:
		void deleteName() {
			switch( type ) {
			case MNT_GL_BUFFER:
			case MNT_GL_TEXTURE:
				// TODO special handling?

			default:
				clReleaseMemObject( name );
				break;
			}
		}

		Name				name;
		MEMOBJ_NAME_TYPE	type;
	};
}

#endif //WIERD_CL_MEMORYOBJECT_H
