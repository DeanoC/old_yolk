// 
//  swfcontrolelement.h
//  SwfParser
//  
//  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
// 
#pragma once

#ifndef YOLK_SWFPARSER_CONTROLELEMENT_H_
#define YOLK_SWFPARSER_CONTROLELEMENT_H_

namespace Swf {

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
}

#endif
