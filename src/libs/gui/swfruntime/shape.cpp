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
#include "swfman.h"
#include "shape.h"

namespace Swf {
	Shape::~Shape() {
		for( auto i : fills ) {
			CORE_DELETE( i );
		}
		for( auto i : lineFills ) {
			CORE_DELETE( i );
		}
		for( auto i : gpuPaths ) {
			CORE_DELETE( i );
		}
		for( auto i : gpuLinePaths ) {
			CORE_DELETE( i );
		}
	}

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
		_ctx->getConstantCache().setWorldMatrix( mat );
		_ctx->getConstantCache().setUserMatrix0( Convert(_colourTransform ) );
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
			if( *i == nullptr) { continue; }
			(*i)->fillStyle->apply( _ctx, _colourTransform, *i );
		}
	}	
	void Shape::displayFill(	Scene::RenderContext* _ctx, 
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph ) {
		for(	GpuPathArray::const_iterator i = gpuPaths.begin();
				i != gpuPaths.end();
				++i ) {
			if( *i == nullptr) { continue; }
			(*i)->fillStyle->apply( _ctx, _colourTransform, *i );
		}
	}
    
} /* Swf */
