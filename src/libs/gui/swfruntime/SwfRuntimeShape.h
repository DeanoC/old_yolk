// 
//  SwfRuntimeShape.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFRUNTIMESHAPE_H_
#define _SWFRUNTIMESHAPE_H_

#include "SwfRuntimeCharacter.h"

namespace Swf {
	// forward decl
	class SwfRuntimeFillStyle;
	class SwfShape;
	class SwfBasePath;
	class SwfPlayer;
	class SwfColourTransform;
	
	// ==========================
	// = The main runtime shape =
	// ==========================
	class SwfRuntimeShape : public SwfRuntimeCharacter {
	public:
        SwfRuntimeShape(SwfShape* _shape) {
            shape = _shape;
        }

        SwfShape* shape;
        std::vector<SwfRuntimeFillStyle*> fills;
        std::vector<SwfRuntimeFillStyle*> lineFills;

		typedef std::vector<SwfBasePath*> GpuPathArray;
        GpuPathArray gpuPaths;
        GpuPathArray gpuLinePaths;

		virtual ~SwfRuntimeShape (){};

        virtual void Display(	SwfPlayer* _player, 
								SwfFrameItem* _parent,
								Math::Matrix4x4* _concatMatrix, 
								SwfColourTransform* _colourTransform, 
								uint16_t _depth, bool _clipLayer, float _morph);

	private:
        void DisplayFill(	SwfPlayer* _player, 
							Math::Matrix4x4* _concatMatrix, 
							SwfColourTransform* _colourTransform, 
							uint16_t _depth, bool _clipLayer, float _morph);
        void DisplayLine(	SwfPlayer* _player, 
							Math::Matrix4x4* _concatMatrix, 
							SwfColourTransform* _colourTransform, 
							uint16_t _depth, bool _clipLayer, float _morph);
	};
} /* Swf */


#endif /* _SWFRUNTIMESHAPE_H_ */


