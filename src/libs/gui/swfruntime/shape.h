// 
//  SwfRuntimeShape.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_SHAPE_H_
#define YOLK_GUI_SWFRUNTIME_SHAPE_H_

#include "character.h"

namespace Swf {
	// forward decl
	class SwfShape;
	class SwfColourTransform;
	class FillStyle;
	class BasePath;
	class Player;
	
	// ==========================
	// = The main runtime shape =
	// ==========================
	class Shape : public Character {
	public:
        Shape( SwfShape* _shape ) {
            shape = _shape;
        }
		virtual ~Shape();

        SwfShape* shape;
        std::vector<FillStyle*> fills;
        std::vector<FillStyle*> lineFills;

		typedef std::vector<BasePath*> GpuPathArray;
        GpuPathArray gpuPaths;
        GpuPathArray gpuLinePaths;

        virtual void display(	Player* _player,
								Scene::RenderContext* _ctx, 
								FrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph) override;

	private:
        void displayFill(	Scene::RenderContext* _ctx,  
							Math::Matrix4x4* _concatMatrix, 
							SwfColourTransform* _colourTransform, 
							uint16_t _depth, bool _clipLayer, float _morph );
        void displayLine(	Scene::RenderContext* _ctx,
							Math::Matrix4x4* _concatMatrix, 
							SwfColourTransform* _colourTransform, 
							uint16_t _depth, bool _clipLayer, float _morph );
	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_SHAPE_H_ */


