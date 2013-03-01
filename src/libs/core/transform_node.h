//!-----------------------------------------------------
//!
//! \file transform_node.h
//! A transform node is a nide with position, orientation
//! and scale, it also can have parent and children 
//! transform node, making a hierachy.
//! A transform node, cannot live without a matrix to
//! create into, this is passed as the constructor param
//! by the owner object. The main reason for the external
//! matrix is for bones
//!
//! NOTE: TransformNode have no ownership semantics,
//!       removing will not delete anything
//!
//!-----------------------------------------------------

#pragma once

#ifndef WIERD_CORE_TRANSFORM_NODE_H
#define WIERD_CORE_TRANSFORM_NODE_H


#include "core/vector_math.h"


namespace Core
{


class TransformNode
{
public:
	typedef std::vector< TransformNode* > NodeContainer;

public:
	std::string m_nodeName;

public:
	//! ctor, the matrix is where the transform will be placed
	TransformNode( Math::Matrix4x4& result );

	//! set the local position of the transform
	void setLocalPosition( const Math::Vector3& pos );
	//! get the local position of the transform
	const Math::Vector3& getLocalPosition() const;
	//! set the local orientation of this node
	void setLocalOrientation( const Math::Quaternion& orient );
	//! get the local orientation of this node
	const Math::Quaternion& getLocalOrientation() const;
	//! set the local scale of the transform
	void setLocalScale( const Math::Vector3& scale );
	//! get the local scale of the transform
	const Math::Vector3& getLocalScale() const;

	//! set all the local transform parameters in one go for speed
	void setLocalTransform( const Math::Vector3& pos, const Math::Quaternion& orient, const Math::Vector3& scale );

	//! returns the transform on this node in local space NOTE: This is compartively expensive (not cached)
	const Math::Matrix4x4 getLocalMatrix() const;

	//! return the world transform of this node
	const Math::Matrix4x4& getWorldMatrix() const;

	const NodeContainer& getNodeContainer() const {
		return children;
	}

	unsigned int getChildCount() const;

	TransformNode* getChild( unsigned int index ) const;

	void addChild( TransformNode* child );

	void removeChild( TransformNode* child );

	TransformNode* getParent();
	const TransformNode* getParent() const;

	// draws using debug lines the node axis and its children
	void debugDisplay() const;

	const Math::Matrix4x4& getRenderMatrix() const { return renderMatrix; }
	void setRenderMatrix();

protected:
	void setParent( TransformNode* parent );

	void dirtyTransforms();

	//! local position
	Math::Vector3		position;
	//! local orientation
	Math::Quaternion	orientation;
	//! local scale
	Math::Vector3		scale;
	//! cached transform matrix made when needed if position and orientation have changed
	mutable Math::Matrix4x4*	transform;
	//! flag for whether to transform matrix is currently valid
	mutable bool				transformCached;
	Math::Matrix4x4				renderMatrix; // double buffer world transform


	TransformNode*		parent;
	NodeContainer		children;

};


}	//namespace Core

#endif
