//!-----------------------------------------------------
//!
//! \file transform_node.cpp
//! see transform_node.h for details
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "debug_render.h"

namespace
{
	class DefaultDRI : public Core::DebugRenderInterface
	{
	public:
		DefaultDRI()
		{
			Core::g_pDebugRender = this;
		}
		//! draw a NDC (projection) space line (resolution independent)
		virtual void ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ){};
		//! draw a 3D world space line
		virtual void worldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b ){};

		//! print some text onto some form of screen console
		virtual void print( const char* pText )
		{
			// if we get here we don't have a screen console so use the Log as a fallback
			LOG(INFO) << pText << "\n";
		};

		//! world sphere
		virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& localPos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() ){};
		//! world cylinder
		virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() ){};
		//! world box
		virtual void worldBox( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() ){};

	} g_DefaultDebugRenderInterface;

}


namespace Core
{
	DebugRenderInterface* g_pDebugRender = 0;
}