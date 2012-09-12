// 
//  SwfRuntimeShape.cpp
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"

#include "SwfPlayer.h"
#include "gui/SwfParser/SwfColourTransform.h"
#include "SwfBasePath.h"
#include "SwfGpuPath.h"
#include "SwfRuntimeFillStyle.h"
#include "SwfRuntimeUtils.h"
/*
#if PLATFORM == IPHONE
#include <OpenGLES/ES1/gl.h>
#elif PLATFORM == APPLE_MAC
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif
*/
#include "SwfRuntimeShape.h"

namespace Swf {
	void SwfRuntimeShape::Display(	SwfPlayer* _player, 
								SwfFrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) {
		
		// early out colour transforms that have made the shape totally invisible							
		if( (_colourTransform->mul[3] < 1e-2f) && 
			(_colourTransform->add[3] < 1e-2f) ){
			return;
		}
		/*
		CALL_GL(glMatrixMode(GL_MODELVIEW));
		Math::Matrix4x4 mat(*_concatMatrix);
		mat._43 = (float)_depth / 255.f;//65535.0f;
		CALL_GL(glLoadMatrixf( mat ));
		*/
		TODO_ASSERT( false && "GL code" );

		DisplayFill( _player, _concatMatrix, _colourTransform, _depth, false, _morph);		
		DisplayLine( _player, _concatMatrix, _colourTransform, _depth, false, _morph);		
	}
	void SwfRuntimeShape::DisplayLine(	SwfPlayer* _player, 
									Math::Matrix4x4* _concatMatrix, 
									SwfColourTransform* _colourTransform, 
									uint16_t _depth, bool _clipLayer, float _morph) {		
		for(	GpuPathArray::const_iterator i = gpuLinePaths.begin();
				i != gpuLinePaths.end();
				++i ) {
			const SwfBasePath* path = *i;
			if( path != NULL ) {
				/*
				// bind vertex and index buffers
				CALL_GL( glBindBuffer( GL_ARRAY_BUFFER, path->vertexBuffer) );
				CALL_GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, path->indexBuffer) );
				CALL_GL( glVertexPointer(2, GL_FLOAT, 0, 0) );
				CALL_GL(glEnableClientState(GL_VERTEX_ARRAY));
				
				// set textures/material setting
				SwfRuntimeFillStyle::APPLY_RESULT ao = path->fillStyle->Apply( _colourTransform );
				if(ao == SwfRuntimeFillStyle::NO_OUTPUT)
					continue;
				
				CALL_GL( glDisable( GL_CULL_FACE ) );
				CALL_GL( glColorMask(1,1,1,0) );
			
				if( ao == SwfRuntimeFillStyle::BLEND_OUTPUT) {
					CALL_GL( glEnable( GL_BLEND ) );
					CALL_GL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
				}
				CALL_GL( glDrawElements(GL_LINES, path->numIndices,  GL_UNSIGNED_SHORT, 0) );

				// restore state
				CALL_GL( glActiveTexture(GL_TEXTURE0) );
				CALL_GL( glDisable(GL_TEXTURE_2D) );
				CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );
				CALL_GL( glActiveTexture(GL_TEXTURE1) );
				CALL_GL( glDisable(GL_TEXTURE_2D) );
				CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );				
				CALL_GL( glDisable(GL_BLEND) );
				CALL_GL( glDisableClientState(GL_TEXTURE_COORD_ARRAY) );
				*/
				TODO_ASSERT( false && "GL code" );
			}
		}
	}	
	void SwfRuntimeShape::DisplayFill(	SwfPlayer* _player, 
									Math::Matrix4x4* _concatMatrix, 
									SwfColourTransform* _colourTransform, 
									uint16_t _depth, bool _clipLayer, float _morph) {
		for(	GpuPathArray::const_iterator i = gpuPaths.begin();
				i != gpuPaths.end();
				++i )
		{
			const SwfBasePath* path = *i;
			if( path != NULL ){
				if( path->IsSimple() ) {
					/*
					// bind vertex and index buffers
					CALL_GL(glBindBuffer( GL_ARRAY_BUFFER, path->vertexBuffer));
					CALL_GL(glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, path->indexBuffer));
					CALL_GL(glVertexPointer(2, GL_FLOAT, 0, 0));
					CALL_GL(glEnableClientState(GL_VERTEX_ARRAY));
					
					// set textures/material setting
					SwfRuntimeFillStyle::APPLY_RESULT ao = path->fillStyle->Apply( _colourTransform );
					if(ao == SwfRuntimeFillStyle::NO_OUTPUT)
						continue;
											
					CALL_GL( glDisable( GL_CULL_FACE ) );
					CALL_GL( glColorMask(1,1,1,0) );
					if( ao == SwfRuntimeFillStyle::BLEND_OUTPUT) {
						CALL_GL( glEnable( GL_BLEND ) );
						CALL_GL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
					}
					CALL_GL( glDrawElements(GL_TRIANGLES, path->numIndices,  GL_UNSIGNED_SHORT, 0) );

					// restore state
					CALL_GL( glActiveTexture(GL_TEXTURE0) );
					CALL_GL( glDisable(GL_TEXTURE_2D) );
					CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );
					CALL_GL( glActiveTexture(GL_TEXTURE1) );
					CALL_GL( glDisable(GL_TEXTURE_2D) );
					CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );
					CALL_GL( glDisable(GL_BLEND) );
					CALL_GL( glDisableClientState(GL_TEXTURE_COORD_ARRAY) );
					*/
					TODO_ASSERT( false && "GL code" );
				} else {
					SwfRuntimeFillStyle::APPLY_RESULT ao = path->fillStyle->TestApply( _colourTransform );
					
					if(ao == SwfRuntimeFillStyle::NO_OUTPUT)
						continue;
					/*
					
					// bind vertex and index buffers
					CALL_GL( glBindBuffer( GL_ARRAY_BUFFER, path->vertexBuffer) );
					CALL_GL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, path->indexBuffer) );
					CALL_GL( glVertexPointer(2, GL_FLOAT, 0, 0) );
					
					// GPU tesselate into Dest.A
					CALL_GL( glDisable( GL_CULL_FACE ) );
					CALL_GL( glEnable( GL_COLOR_LOGIC_OP) );
					CALL_GL( glLogicOp( GL_XOR ) );
					CALL_GL( glColorMask(0,0,0,1) );
					CALL_GL( glColor4f(0.0f,0.0f,0.0f,1.0f) );
					CALL_GL( glDrawElements(GL_TRIANGLES, path->numIndices,  GL_UNSIGNED_SHORT, 0) );
				
					// set textures/material setting
					path->fillStyle->Apply( _colourTransform );
					CALL_GL( glDisable( GL_COLOR_LOGIC_OP ) );
					CALL_GL( glEnable( GL_BLEND ) );
				
					if( ao == SwfRuntimeFillStyle::BLEND_OUTPUT) {
						// Dest.A = Dest.A * Material.Alpha
						CALL_GL( glBlendFunc( GL_ZERO, GL_SRC_ALPHA ) );
						CALL_GL( glDrawArrays(GL_TRIANGLE_FAN, path->extentsRectVertexOffset, 4) );
					}
					
					// Dest.colour = Material.colour * Dest.Alpha (which has tesselation + material alpha)
					CALL_GL( glColorMask(1,1,1,1) );
					CALL_GL( glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA ) );
					CALL_GL( glDrawArrays(GL_TRIANGLE_FAN, path->extentsRectVertexOffset, 4) );
			
					// ensure dest alpha is cleared for this polygon
					CALL_GL( glDisable(GL_BLEND) );
					CALL_GL( glColorMask(0,0,0,1) );
					CALL_GL( glEnable( GL_COLOR_LOGIC_OP) );
					CALL_GL( glLogicOp( GL_CLEAR ) );
					CALL_GL( glDrawArrays(GL_TRIANGLE_FAN, path->extentsRectVertexOffset, 4) );
					
					// restore state
					CALL_GL( glColorMask(1,1,1,1) );
					CALL_GL( glActiveTexture(GL_TEXTURE0) );
					CALL_GL( glDisable(GL_TEXTURE_2D) );
					CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );
					CALL_GL( glActiveTexture(GL_TEXTURE1) );
					CALL_GL( glDisable(GL_TEXTURE_2D) );
					CALL_GL( glBindTexture(GL_TEXTURE_2D, 0) );
					CALL_GL( glDisableClientState(GL_TEXTURE_COORD_ARRAY) );
					CALL_GL( glDisable(GL_BLEND) );
					CALL_GL( glDisable( GL_COLOR_LOGIC_OP ) );
					*/
					TODO_ASSERT( false && "GL code" );
				}
			}
		}
	}
    
} /* Swf */
