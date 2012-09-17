/*
 *  SwfFramaItem.cpp
 *  SwfPreview
 *
 *  Created by Deano on 27/07/2009.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "swfruntime.h"
#include "ActionScript/autogen/AsObject.h"
#include "frameitem.h"

namespace Swf {
	
FrameItem::AsPropertyStringMap* FrameItem::s_asPropertyStringMap;
	
float FrameItem::getXScale() {
	return sqrt(concatMatrix._11 * concatMatrix._11 + concatMatrix._12 * concatMatrix._12);
}

void FrameItem::setXScale( float _xscale ) {
	float oldScale = getXScale();
	concatMatrix._11 = (concatMatrix._11 / oldScale) * _xscale;
	concatMatrix._12 = (concatMatrix._12 / oldScale) * _xscale;
}

float FrameItem::getYScale() {
	return sqrt(concatMatrix._22 * concatMatrix._22 + concatMatrix._21 * concatMatrix._21);
}

void FrameItem::setYScale( float _yscale ) {
	float oldScale = getYScale();
	concatMatrix._22 = (concatMatrix._22 / oldScale) * _yscale;
	concatMatrix._21 = (concatMatrix._21 / oldScale) * _yscale;
}

float FrameItem::getRotation() {
	return Math::radian_to_degree<float>() * atan2(concatMatrix._21, concatMatrix._11);
}

void FrameItem::setRotation(float angle) {
	angle = Math::degree_to_radian<float>() * angle;
	float xscale = getXScale();
	float yscale = getYScale();
	float cosAngle = cos(angle);
	float sinAngle = sin(angle);
	concatMatrix._11 = (float)(xscale * cosAngle);
	concatMatrix._21 = (float)(yscale * -sinAngle);
	concatMatrix._12 = (float)(xscale * sinAngle);
	concatMatrix._22 = (float)(yscale * cosAngle);
}

Swf::AutoGen::AsObjectHandle FrameItem::getProperty( int _index ) {
	using namespace AutoGen;
	
	switch( _index) {
		case 0: // _X
		case 1: // _Y
		break;
		case 2: //_xscale 2 
			return CORE_NEW AsObjectNumber( getXScale() );
		case 3: //_yscale 3 
			return CORE_NEW AsObjectNumber( getYScale() );
		case 4: //_currentframe 4 
		case 5: //_totalframes 5 
		case 6: //_alpha 6 
		break;
		case 7: //_visible 7
			return CORE_NEW AsObjectBool( visible );
		case 8: // _width 8 
		case 9: //_height 9 
		break;
		case 10: //_rotation 10 
			return CORE_NEW AsObjectNumber( getRotation() );
		case 11: //_target 11 
		case 12: //_framesloaded 12 
		break;
		case 13: //_name 13 
			return CORE_NEW AsObjectString( name );
		case 14: //_droptarget 14 
		case 15: //_url 15			
		case 16: //_highquality 16 
		case 17: //_focusrect 17 
		case 18: //_soundbuftime 18 
		case 19: //_quality 19 
		case 20: //_xmouse 20 
		case 21: //_ymouse 21 
		break;
	}
	return AsObjectUndefined::Get();
};

void FrameItem::setProperty( int _index, AutoGen::AsObjectHandle _val ){	
	switch( _index ) {
		case 0: // _X
		case 1: // _Y
		break;
		case 2: //_xscale 2 
			setXScale( (float)_val->ToNumber() ); break;
		case 3: //_yscale 3 
			setYScale( (float)_val->ToNumber() ); break;
		case 4: //_currentframe 4 
		case 5: //_totalframes 5 
		case 6: //_alpha 6
		break;
		case 7: //_visible 7 
			visible = _val->ToBoolean(); break;
		case 8: // _width 8 
		case 9: //_height 9 
		break;
		case 10: //_rotation 10 
			setRotation( (float)_val->ToNumber() ); break;
		case 11: //_target 11 
		case 12: //_framesloaded 12 
		break;
		case 13: //_name 13 
			name = _val->ToString(); break;
		case 14: //_droptarget 14 
		case 15: //_url 15			
		case 16: //_highquality 16 
		case 17: //_focusrect 17 
		case 18: //_soundbuftime 18 
		case 19: //_quality 19 
		case 20: //_xmouse 20 
		case 21: //_ymouse 21 
		break;
	}
};


AutoGen::AsObjectHandle FrameItem::getProperty( const std::string& _name ) {
	using namespace AutoGen;
	
	AsPropertyStringMap::const_iterator propIt = s_asPropertyStringMap->find( _name );
	if( propIt != s_asPropertyStringMap->end() ) {
		return AsObjectHandle( getProperty( propIt->second ) );
	} else {
		return AsObject::GetProperty( _name );
	}
}

void FrameItem::setProperty( const std::string& _name, AutoGen::AsObjectHandle _handle ) {
	using namespace AutoGen;

	CORE_ASSERT( s_asPropertyStringMap && "SWF Manager singleton needs createing" );
	
	AsPropertyStringMap::iterator propIt = s_asPropertyStringMap->find( _name );
	if( propIt != s_asPropertyStringMap->end() ) {
		setProperty( propIt->second, _handle );
	} else {
		AsObject::SetProperty( _name, _handle );
	}
}

} /* Swf */ 


