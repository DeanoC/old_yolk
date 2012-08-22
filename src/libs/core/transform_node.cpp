//!-----------------------------------------------------
//!
//! \file transform_node.cpp
//! see transform_node.h for details
//!
//!-----------------------------------------------------

#include "core/core.h"
#include "transform_node.h"
#include "core/debug_render.h"

namespace Core
{

TransformNode::TransformNode( Math::Matrix4x4& result ) :
	position( Math::Vector3(0,0,0) ),
	orientation( Math::IdentityQuat() ),
	scale( Math::Vector3(1,1,1) ),
	transform( &result ),
	transformCached( false ),
	parent( 0 ) {
}

void TransformNode::setLocalPosition( const Math::Vector3& pos ) {
	position = pos;
	dirtyTransforms();
}

const Math::Vector3& TransformNode::getLocalPosition() const {
	return position;
}

void TransformNode::setLocalOrientation( const Math::Quaternion& orient ) {
	orientation = orient;
	dirtyTransforms();
}

const Math::Quaternion& TransformNode::getLocalOrientation() const {
	return orientation;
}

void TransformNode::setLocalScale( const Math::Vector3& scal ) {
	scale = scal;
	dirtyTransforms();
}

const Math::Vector3& TransformNode::getLocalScale() const {
	return scale;
}

void TransformNode::setLocalTransform( const Math::Vector3& pos, const Math::Quaternion& orient, const Math::Vector3& scal ) {
	position = pos;
	orientation = orient;
	scale = scal;

	dirtyTransforms();
}

const Math::Matrix4x4 TransformNode::getLocalMatrix() const {
	Math::Matrix4x4 transMat = Math::CreateTranslationMatrix( position );
	Math::Matrix4x4 rotMat = Math::CreateRotationMatrix( orientation );
	Math::Matrix4x4 scaleMat = Math::CreateScaleMatrix( scale );

	return rotMat * scaleMat * transMat;
}

const Math::Matrix4x4& TransformNode::getWorldMatrix() const {
	if( !transformCached ) {
		*transform = getLocalMatrix();
		if( parent != 0  ) {
			*transform = parent->getWorldMatrix() * *transform;
		}
		transformCached = true;
	} 

	return *transform;

}
void TransformNode::setParent( TransformNode* _parent ) {
	CORE_ASSERT( _parent != this );
	parent = _parent;
}

TransformNode* TransformNode::getChild( unsigned int index ) const {
	return children[index];
}

unsigned int TransformNode::getChildCount() const {
	return (unsigned int) children.size();
}

void TransformNode::addChild( TransformNode* child ) {
	CORE_ASSERT( child->parent == 0 );
	CORE_ASSERT( child != this );
	children.push_back( child );
	child->parent = this;
}

void TransformNode::removeChild( TransformNode* child ) {
	NodeContainer::iterator chIt = std::find( children.begin(), children.end(),  child );
	CORE_ASSERT( chIt != children.end() && "node isn't my child" );
	CORE_ASSERT( (child->parent == this) && "node isn't my child" );
	child->parent = 0;
	children.erase( chIt );
}

TransformNode* TransformNode::getParent() {
	return parent;
}
const TransformNode* TransformNode::getParent() const {
	return parent;
}

void TransformNode::dirtyTransforms() {
	transformCached = false;

	NodeContainer::const_iterator chIt = children.begin();
	while( chIt != children.end() ) {
		(*chIt)->dirtyTransforms();
		++chIt;
	}
}

void TransformNode::setRenderMatrix() { 
	renderMatrix = getWorldMatrix();

	NodeContainer::const_iterator chIt = children.begin();
	while( chIt != children.end() ) {
		(*chIt)->setRenderMatrix();
		++chIt;
	}
}

void TransformNode::debugDisplay() const {
	// draw local axis
	float axislength = 1.0f;
	const Math::Matrix4x4& matworld = getWorldMatrix();
	g_pDebugRender->worldLine( Core::RGBAColour(1,0,0,1), Math::GetTranslation(matworld), Math::GetTranslation(matworld)+Math::GetXAxis(matworld)*axislength );
	g_pDebugRender->worldLine( Core::RGBAColour(0,1,0,1), Math::GetTranslation(matworld), Math::GetTranslation(matworld)+Math::GetYAxis(matworld)*axislength );
	g_pDebugRender->worldLine( Core::RGBAColour(0,0,1,1), Math::GetTranslation(matworld), Math::GetTranslation(matworld)+Math::GetZAxis(matworld)*axislength );

	// draw connecting line to parent
	if( getParent() != NULL ) {
		const Math::Matrix4x4& matworldparent = getParent()->getWorldMatrix();
		g_pDebugRender->worldLine( Core::RGBAColour(1,1,1,1), Math::GetTranslation(matworld), Math::GetTranslation(matworldparent) );
	}

	// tell children to draw themselves
	NodeContainer::const_iterator chIt = children.begin();
	while( chIt != children.end() ) {
		(*chIt)->debugDisplay();
		++chIt;
	}
}


}
