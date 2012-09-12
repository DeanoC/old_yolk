// 
//  SwfMatrix.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#ifndef _SWFMATRIX_H_
#define _SWFMATRIX_H_

namespace Swf
{
	// forward decl
	class SwfStream;
	
	class SwfMatrix
	{
	public:
		SwfMatrix (){
			scaleX = 1.0f;
			rotateSkew0 = 0.0f;
			rotateSkew1 = 0.0f;
			scaleY = 1.0f;
			translateX = 0.0f;
			translateY = 0.0f;
		};
		virtual ~SwfMatrix (){};
        SwfMatrix(float _scaleX, float _rotateSkew0, float _rotateSkew1, float _scaleY, float _translateX, float _translateY)
        {
            scaleX = _scaleX;
            rotateSkew0 = _rotateSkew0;
            rotateSkew1 = _rotateSkew1;
            scaleY = _scaleY;
            translateX = _translateX;
            translateY = _translateY;
        }
		static SwfMatrix* Read(SwfStream& _stream);

		float scaleX;
		float rotateSkew0;
		float rotateSkew1;
		float scaleY;
		float translateX;
		float translateY;
	};
} /* Swf */


#endif /* _SWFMATRIX_H_ */

