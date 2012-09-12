// 
//  SwfClipActions.h
//  SwfParser
//  
//  Created by Deano on 2008-09-25.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFCLIPACTIONS_H_
#define _SWFCLIPACTIONS_H_

namespace Swf
{
	// forward Decl
	class SwfStream;
    enum ClipEventFlags
    {
        KeyUp = 0x1,
        KeyDown = 0x2,
        MouseUp = 0x4,
        MouseDown = 0x8,
        MouseMove = 0x10,
        Unload = 0x20,
        Load = 0x40,
        DragOver = 0x80,
        RollOut = 0x100,
        RollOver = 0x200,
        ReleaseOutside = 0x400,
        Release = 0x800,
        Press = 0x1000,
        Initialize = 0x2000,
        Data = 0x4000,
        Reserved_0 = 0x8000,
        Reserved_1 = 0x10000,
        Reserved_2 = 0x20000,
        Reserved_3 = 0x40000,
        Reserved_4 = 0x80000,
        Construct = 0x100000,
        KeyPress = 0x200000,
        DragOut = 0x400000,
	};

	class SwfClipActionRecord
	{
	public:
		SwfClipActionRecord() :
			eventFlags((ClipEventFlags)0),
			keyCode(0),
			byteCode(0)
		{}
		
		ClipEventFlags eventFlags;
		uint8_t keyCode;
		uint8_t* byteCode; // this is the raw action script byte code
		static SwfClipActionRecord* Read(SwfStream& _stream);
	};
	
	class SwfClipActions
	{
	public:
		SwfClipActions() : 
			allEventFlags((ClipEventFlags)0),
			clipActionRecords(0)
		{}
		
		ClipEventFlags allEventFlags;
		SwfClipActionRecord** clipActionRecords;
		static SwfClipActions* Read(SwfStream& _stream);
	};
    
} /* Swf */


#endif /* _SWFCLIPACTIONS_H_ */

