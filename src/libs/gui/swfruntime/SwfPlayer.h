// 
//  SwfPlayer.h
//  Projects
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_SWFPLAYER_H_
#define YOLK_GUI_SWFRUNTIME_SWFPLAYER_H_

#include "core/resources.h"

namespace Scene {
	class ResourceLoader;
}

namespace Swf {
	// forward decl
	class Parser;
	class SwfMovieClip;
	class SwfRuntimeCharacter;
	class SwfGradientTextureManager;
	class SwfRuntimeBitmap;
	class SwfRuntimeFont;
#if defined( USE_ACTIONSCRIPT )
	class AsVM;
#endif	
	
	enum STOCK_TEXTURE {
		OPAQUE_WHITE_TEXTURE = 0,
		OPAQUE_BLACK_TEXTURE,
		
		NUM_STOCK_TEXTURES,
	};

	static const uint32_t SwfPlayerType = RESOURCE_NAME('S','W','F','P');

	typedef std::map<uint16_t, SwfRuntimeCharacter*>	CharacterPaths;
	typedef std::map<uint16_t, SwfRuntimeBitmap*>		RuntimeBitmaps;
	typedef std::map<uint16_t, SwfRuntimeFont*> 		RuntimeFonts;
		
	// =================================
	// = Main interface to a Swf Movie =
	// =================================
	class SwfPlayer : public Core::GcBase, public Core::Resource<SwfPlayerType> {
	public:
		friend class Scene::ResourceLoader;
		struct CreationInfo {};

		SwfPlayer();
				
		virtual ~SwfPlayer ();

		void create( const std::string& _path, float _xscale, float _yscale );
		void advance( float _elapsedMs );
		void togglePause();
		void display( int _backingWidth, int _backingHeight );
		void process();

		SwfRuntimeCharacter* getCharacter(uint16_t _id) { return characterPaths[ _id ]; }
		SwfRuntimeBitmap* getBitmap(uint16_t _id) { return runtimeBitmaps[ _id ]; }
		SwfRuntimeFont* getFont( uint16_t _id ) { return runtimeFonts[ _id ]; }

		// TODO make stock path static? or not needed once resource system fully integrated
		void setStockPath( const std::string& _stockPath ) { stockPath = _stockPath; }
		const std::string& getStockPath() const { return stockPath; }
				
		Parser* parser;
		float frameRateInMs;
		SwfGradientTextureManager* getGradientTextureManager() { return gradientTextureManager; }
		unsigned int getStockTexture( STOCK_TEXTURE _tex ) const { return stockTextures[ _tex ]; }
		
		const Math::Matrix4x4& getTwipToNdx() const { return twipToNdx; }
		const Math::Matrix4x4& getTwipToPixels() const { return twipToPixels; }
		
#if defined( USE_ACTIONSCRIPT )
		AsVM*				virtualMachine;
#endif
		
	private:
		static const void* internalPreCreate( const char* name, const CreationInfo *creation );

		unsigned int stockTextures[NUM_STOCK_TEXTURES];
		std::string							stockPath;
		
		float								scaleX;
		float								scaleY;
		bool								paused;
		SwfMovieClip*						rootClip;
		CharacterPaths						characterPaths;
		RuntimeBitmaps						runtimeBitmaps;
		RuntimeFonts						runtimeFonts;
		
		SwfGradientTextureManager*			gradientTextureManager;
		Math::Matrix4x4 					twipToNdx;
		Math::Matrix4x4 					twipToPixels;
	};

	typedef const Core::ResourceHandle<SwfPlayerType, SwfPlayer> SwfPlayerHandle;
	typedef SwfPlayerHandle* SwfPlayerHandlePtr;
	typedef std::shared_ptr<SwfPlayer> SwfPlayerPtr;

} /* Swf */


#endif /* _SWFPLAYER_H_ */
