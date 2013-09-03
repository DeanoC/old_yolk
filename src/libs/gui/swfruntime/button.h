#ifndef YOLK_GUI_SWFRUNTIME_BUTTON_H_
#define YOLK_GUI_SWFRUNTIME_BUTTON_H_

/*
 *  button.h
 *
 *  Copyright 2013 Cloud Pixies Ltd. All rights reserved.
 *
 */

#include "frameitem.h"
#include "displayobjectframeitem.h"

namespace Swf {
	// forward decl
	class SwfFrame;
	class SwfDisplayObject;
	class SwfButton;
	class Player;
	struct SwfButtonRecord;
	struct SwfButtonCondAction;

	// =====================================================
	// = Button has several sprites in side based on state =
	// =====================================================
	class Button : public DisplayObjectFrameItem {
	public:
		Button( SwfDisplayObject* _dobj, SwfButton* _button, MovieClip* _parent );
		virtual ~Button ();

		enum STATE {
			IDLE		=		0,
			DOWN		=		BIT(0),
			OVER		=		BIT(1),
			MAX_STATES	=		4
		};

		void updateState( Player* _player, const Math::Vector2& mouseTwip, bool leftButton, bool rightButton );
		void display( const Player* _player, Scene::RenderContext* _ctx ) override;

	private:
		bool testHitRecord( const Player* _player, const Math::Vector2& _pt );

		void displayRecord( int _recI, const Player* _player, Scene::RenderContext* _ctx );

		SwfButtonRecord* hitRecord;
		SwfButton* button;
		STATE currentState;

		SwfButtonCondAction* transitionTable[MAX_STATES][MAX_STATES];
	};
	
}
#endif /* end of include guard: YOLK_GUI_SWFRUNTIME_TEXT_H_ */
