// 
//  SwfRuntimeBuilder.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef YOLK_GUI_SWFRUNTIME_BUILDER_H_
#define YOLK_GUI_SWFRUNTIME_BUILDER_H_

#include "player.h"

namespace Swf {
	//forward decl
	class SwfParser;
	class SwfShape;
	class SwfFillStyle;
	class SwfLineStyle;

	class BasePath;
	class Player;
	class Builder;
	class FillStyle;
	class Shape;
	class Character;
	class Bitmap;
	class Font;
		
	class Builder {
	public:
		Builder( Player* _player, SwfParser* _parser ) {
		    player = _player;
		    parser = _parser;
		}

		void buildCharacterPaths( CharacterPaths& _charPaths );
		void buildBitmaps( Bitmaps& _runtimeBitmaps );
		void buildFonts( Fonts& _runtimeFonts );

	private:
		SwfParser* parser;
		Player* player;

		void buildRuntimeShape( Shape* _runShape, SwfShape* _swfShape );

		void buildFillStyles( SwfFillStyle**& _fills, int _numFills, std::vector<FillStyle*>& _rtfills );
		void buildLineStyles( SwfLineStyle**& _fills, bool _morph, int _numLineFills, std::vector<FillStyle*>& _rtLines );

		void buildPath( const SwfShape* _swfShape, 
	                    std::vector< FillStyle* >& _fillStyles, 
	                    std::vector< FillStyle* >& _lineFillStyles,
	                    std::vector< BasePath* >& _fillPaths,
	                    std::vector< BasePath* >& _linePaths,
	                    bool _morphShape,
						bool _morphEnd );
		void generateShapeGpuData( Shape* _runShape );

		enum STOCK_FONT {
			SANS = 0,
			SERIF,
			TYPEWRITER,

			NUM_STOCK_FONTS,
		};

		Font* getStockFont( STOCK_FONT _font );
		Font* loadSpecialFont( std::string _path );

	};
} /* Swf */


#endif /* YOLK_GUI_SWFRUNTIME_BUILDER_H_ */
