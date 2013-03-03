/*
 *  SwfButton.h
 *  SwfPreview
 *
 *  Created by Deano on 13/08/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef SWFBUTTON_H_WL9YSR3A
#define SWFBUTTON_H_WL9YSR3A

#include "SwfStream.h"
#include "SwfCharacter.h"

namespace Swf
{
	class SwfMatrix;
	class SwfColourTransform;
	class SwfActionByteCode;

	struct SwfButtonRecord {
		bool buttonHasBlendMode;
		bool buttonHasFilterList;
		bool buttonStateHitTest;
		bool buttonStateDown;
		bool buttonStateOver;
		bool buttonStateUp;
		uint16_t characterId;
		uint16_t placeDepth;
		SwfMatrix* matrix;
		SwfColourTransform* cxform;
	};

	struct SwfButtonCondAction {
		bool idleToOverDown;
		bool outDownToIdle;
		bool outDownToOverDown;
		bool overDownToOutDown;
		bool overDownToOverUp;
		bool overUpToOverDown;
		bool overUpToIdle;
		bool idleToOverUp;
		bool overDownToIdle;
		uint8_t keyCode;
		SwfActionByteCode* actionScript;
	};

	class SwfButton : public SwfCharacter
	{
	public:
		virtual ~SwfButton();
		static SwfButton* Read( SwfStream& _stream, int _length, int _version );
		
		SwfButton(uint16_t _id) : 	
			SwfCharacter(CT_BUTTON, _id),
			numRecords( 0 ),
			records( nullptr ),
			numCondActions( 0 ),
			condActions( nullptr )
		{}

		int							numRecords;
		SwfButtonRecord*			records;
		int							numCondActions;
		SwfButtonCondAction*		condActions;
	};
	
} /* Swf */ 


#endif /* end of include guard: SWFBUTTON_H_WL9YSR3A */

