// 
//  SwfDisplayObject.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#ifndef _SWFDISPLAYOBJECT_H_
#define _SWFDISPLAYOBJECT_H_

//#include <inttypes.h>
namespace Swf
{
	// forward decl
	class SwfMatrix;
	class SwfColourTransform;
	class SwfClipActions;
	class SwfStream;
	
	enum ControlElementType{
		CE_REMOVEOBJECT,
		CE_DISPLAYOBJECT,
		CE_ACTIONBYTECODE,
	};
	
	class SwfControlElement
	{
	public:
		virtual ~SwfControlElement (){};
		ControlElementType type;
	protected:
		SwfControlElement( ControlElementType _type ) :
			type(_type) {}
	};
	
	class SwfRemoveObject : public SwfControlElement
    {
	public:
		SwfRemoveObject() : 
			SwfControlElement(CE_REMOVEOBJECT),
			depth(0)
		{
			
		}
		uint16_t depth;

		static SwfRemoveObject* Read(SwfStream& _stream, int _removeVer);		
	};
	
	class SwfDisplayObject : public SwfControlElement
    {
	public:
		SwfDisplayObject() :
			SwfControlElement(CE_DISPLAYOBJECT),
			hasClipDepth(false),
			hasMorphRatio(false),
			id(0),
			matrix(0),
			cxform(0),
			morphRatio(0.0f),
			clipDepth(0),
			clipActions(0),
			depth(0),
			hasMatrix(false),
			hasColourTransform(false)
		{
			
		}
        bool hasClipDepth;
        bool hasMorphRatio;
        
        uint16_t id; // only set when adding to the display list (0 = no id)
        SwfMatrix* matrix; // identity matrix if not set from swf stream
        SwfColourTransform* cxform; // identity colour transform if not set from swf stream
        float morphRatio; // if hasMorphRatio == true, this is tween value across a morph
        std::string name; // empty if not set
        uint16_t clipDepth; // if hasClipDepth is set the depth with which a clip layer is set
        SwfClipActions* clipActions; // can be null or empty
        uint16_t depth;
        bool hasMatrix;
        bool hasColourTransform;

		static SwfDisplayObject* Read(SwfStream& _stream, int _swfVersion);

	};
} /* Swf */


#endif /* _SWFDISPLAYOBJECT_H_ */

