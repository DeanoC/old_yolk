/** \file collada.h
	Take collada file off disk into a meshmod object structure
   (c) 2005 Dean Calver
 */

#if !defined(GOIMPORT_COLLADA_H_ )
#define GOIMPORT_COLLADA_H_
#pragma once

class domVisual_scene;

// takes a collada visual scene and returns a GO scene
MeshMod::SceneNodePtr ColladaVisualScene2GO( const domVisual_scene* pNode );
void FreeCollada( DAE& dae );
#endif