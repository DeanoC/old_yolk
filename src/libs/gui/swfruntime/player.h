// 
//  SwfPlayer.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_PLAYER_H_
#define YOLK_GUI_SWFRUNTIME_PLAYER_H_

#include "core/resources.h"

namespace Scene {
	class ResourceLoader;
	class RenderContext;
}

namespace Swf {
	// forward decl
	class SwfParser;
	class MovieClip;
	class Character;
	class GradientTextureManager;
	class Bitmap;
	class Font;
	class AsVM;

	static const uint32_t PlayerType = RESOURCE_NAME('S','W','F','P');

	typedef std::map<uint16_t, Character*>				CharacterPaths;
	typedef std::map<uint16_t, Bitmap*>					Bitmaps;
	typedef std::map<uint16_t, Font*> 					Fonts;
		
	// =================================
	// = Main interface to a Swf Movie =
	// =================================
	class Player : public Core::GcBase, public Core::Resource<PlayerType> {
	public:
		friend class Scene::ResourceLoader;
		struct CreationInfo {};

		Player();
				
		virtual ~Player ();

		void create( const std::string& _path, float _xscale, float _yscale );
		void advance( float _elapsedMs );
		void togglePause();
		void display( Scene::RenderContext* _ctx );

		Character* getCharacter(uint16_t _id) { return characterPaths[ _id ]; }
		Bitmap* getBitmap(uint16_t _id) { return runtimeBitmaps[ _id ]; }
		Font* getFont( uint16_t _id ) { return runtimeFonts[ _id ]; }
		
		SwfParser* parser;
		float frameRateInMs;

		MovieClip* getRoot() const { return rootClip; }
	
		const Math::Matrix4x4& getTwipToNdx() const { return twipToNdx; }

		// special use
		void process();
		AsVM*				virtualMachine;
	private:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );
	
		float								scaleX;
		float								scaleY;
		bool								paused;
		MovieClip*							rootClip;
		CharacterPaths						characterPaths;
		Bitmaps								runtimeBitmaps;
		Fonts								runtimeFonts;
		
		Math::Matrix4x4 					twipToNdx;
	};

	typedef const Core::ResourceHandle<PlayerType, Player> PlayerHandle;
	typedef PlayerHandle* PlayerHandlePtr;
	typedef std::shared_ptr<Player> PlayerPtr;

} /* Swf */


#endif /* _SWFPLAYER_H_ */
