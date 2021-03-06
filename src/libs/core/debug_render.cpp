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
		virtual void ndcLine( const Core::Colour& colour, const Math::Vector2& a, const Math::Vector2& b ) override {};
		//! draw a 3D world space line
		virtual void worldLine( const Core::Colour& colour, const Math::Vector3& a, const Math::Vector3& b ) override {};

		//! print some text onto some form of screen console
		virtual void print( const char* pText )
		{
			// if we get here we don't have a screen console so use the Log as a fallback
			LOG(INFO) << pText << "\n";
		};
		virtual int getNumberOfVarPrints() const override { return 0; }
		virtual void varPrint( const int _index, const Math::Vector2& _pos, const char* _text ) override {};

		//! world sphere
		virtual void worldSphere( const Core::Colour& colour, const Math::Vector3& localPos, const float radius, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override {};
		//! world cylinder
		virtual void worldCylinder( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float radius, const float height, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override {};
		//! world box
		virtual void worldBox( const Core::Colour& colour, const Math::Vector3& localPos, const Math::Quaternion& localOrient, const float lx, const float ly, const float lz, const Math::Matrix4x4& transform = Math::IdentityMatrix() ) override {};

	} g_DefaultDebugRenderInterface;

}


namespace Core
{
	DebugRenderInterface* g_pDebugRender = 0;
}