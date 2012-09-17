// 
//  SwfRuntimeShape.cpp
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/SwfColourTransform.h"
#include "scene/rendercontext.h"
#include "basepath.h"
#include "fillstyle.h"
#include "gpupath.h"
#include "player.h"
#include "utils.h"
#include "shape.h"

namespace Swf {
	void Shape::display(	Player* _player,
							Scene::RenderContext* _ctx,
							FrameItem* _parent,
							Math::Matrix4x4* _concatMatrix, 
							SwfColourTransform* _colourTransform, 
							uint16_t _depth, bool _clipLayer, float _morph) {
		
		// early out colour transforms that have made the shape totally invisible							
		if( (_colourTransform->mul[3] < 1e-2f) && 
			(_colourTransform->add[3] < 1e-2f) ){
			return;
		}
		Math::Matrix4x4 mat(*_concatMatrix);
		mat._43 = (float)_depth / 255.f;//65535.0f; // TODO code is straight from GL, see why
		_ctx->getConstantCache().changeWorldMatrix( mat );
		_ctx->getConstantCache().updateGPUBlock( _ctx, Scene::CF_STD_OBJECT );

		displayFill( _ctx, _concatMatrix, _colourTransform, _depth, false, _morph );		
		displayLine( _ctx, _concatMatrix, _colourTransform, _depth, false, _morph );		
	}
	void Shape::displayLine(	Scene::RenderContext* _ctx,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph ) {
		for(	GpuPathArray::const_iterator i = gpuLinePaths.begin();
				i != gpuLinePaths.end();
				++i ) {
			const BasePath* path = *i;
			if( path != NULL ) {
				// set textures/material setting
				FillStyle::APPLY_RESULT ao = path->fillStyle->apply( _colourTransform );
				if(ao == FillStyle::NO_OUTPUT)
					continue;

				// bind vertex and index buffers					

				auto vb = path->vertexBufferHandle.tryAcquire();
				if( !vb ) { continue; }
				auto ib = path->indexBufferHandle.tryAcquire();
				if( !ib ) { continue; }
				_ctx->bindVB( 0, vb, sizeof(float) * 2 );
				_ctx->bindIB( ib, sizeof(uint16_t) );
				if( ao == FillStyle::BLEND_OUTPUT) {
//					CALL_GL( glEnable( GL_BLEND ) );
//					CALL_GL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
				}
				_ctx->drawIndexed( Scene::PT_LINE_LIST, path->numIndices );
				/*
				CALL_GL( glDisable( GL_CULL_FACE ) );
				CALL_GL( glColorMask(1,1,1,0) );		
				*/
			}
		}
	}	
	void Shape::displayFill(	Scene::RenderContext* _ctx, 
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph ) {
		for(	GpuPathArray::const_iterator i = gpuPaths.begin();
				i != gpuPaths.end();
				++i )
		{
			const BasePath* path = *i;
			if( path != NULL ){
				if( path->isSimple() ) {
					// set textures/material setting
					FillStyle::APPLY_RESULT ao = path->fillStyle->apply( _colourTransform );
					if(ao == FillStyle::NO_OUTPUT)
						continue;

					// bind vertex and index buffers					
					auto vb = path->vertexBufferHandle.tryAcquire();
					if( !vb ) { continue; }
					auto ib = path->indexBufferHandle.tryAcquire();
					if( !ib ) { continue; }
					_ctx->bindVB( 0, vb, sizeof(float) * 2 );
					_ctx->bindIB( ib, sizeof(uint16_t) );
					if( ao == FillStyle::BLEND_OUTPUT) {
	//					CALL_GL( glEnable( GL_BLEND ) );
	//					CALL_GL( glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
					}
					_ctx->drawIndexed( Scene::PT_TRIANGLE_LIST, path->numIndices );
					/*											
					CALL_GL( glDisable( GL_CULL_FACE ) );
					CALL_GL( glColorMask(1,1,1,0) );
					*/
				} else {
					FillStyle::APPLY_RESULT ao = path->fillStyle->testApply( _colourTransform );
					
					if(ao == FillStyle::NO_OUTPUT)
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
					*/
					TODO_ASSERT( false && "GL code" );
				}
			}
		}
	}
    
} /* Swf */
