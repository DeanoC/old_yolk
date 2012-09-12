// 
//  SwfRuntimeBuilder.h
//  Projects
//  
//  Created by Deano on 2008-09-28.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#ifndef _SWFRUNTIMEBUILDER_H_
#define _SWFRUNTIMEBUILDER_H_

#include "SwfPlayer.h"
namespace Swf {
	//forward decl
	class Parser;
	class SwfPlayer;
	class SwfRuntimeBuilder;
	class SwfShape;
	class SwfRuntimeFillStyle;
	class SwfBasePath;
	class SwfRuntimeShape;
	class SwfFillStyle;
	class SwfLineStyle;
	class SwfRuntimeCharacter;
	class SwfRuntimeBitmap;
	class SwfRuntimeFont;
		
	class SwfRuntimeBuilder {
	public:
		SwfRuntimeBuilder(SwfPlayer* _player, Parser* _parser) {
		    player = _player;
		    parser = _parser;
		}

		void BuildCharacterPaths(CharacterPaths& _charPaths);
		void BuildBitmaps(RuntimeBitmaps& _runtimeBitmaps);
		void BuildFonts(RuntimeFonts& _runtimeFonts);

	private:
		Parser* parser;
		SwfPlayer* player;

		void BuildRuntimeShape(SwfRuntimeShape* _runShape, SwfShape* _swfShape);

		void BuildFillStyles(SwfFillStyle**& _fills, int _numFills, std::vector<SwfRuntimeFillStyle*>& _rtfills );
		void BuildLineStyles(SwfLineStyle**& _fills, bool _morph, int _numLineFills, std::vector<SwfRuntimeFillStyle*>& _rtLines );

		void BuildPath( const SwfShape* _swfShape, 
	                    std::vector<SwfRuntimeFillStyle*>& _fillStyles, 
	                    std::vector<SwfRuntimeFillStyle*>& _lineFillStyles,
	                    std::vector<SwfBasePath*>& _fillPaths,
	                    std::vector<SwfBasePath*>& _linePaths,
	                    bool _morphShape,
						bool _morphEnd );
		void GenerateShapeGpuData(SwfRuntimeShape* _runShape);

		enum STOCK_FONT {
			SANS = 0,
			SERIF,
			TYPEWRITER,

			NUM_STOCK_FONTS,
		};

		SwfRuntimeFont* GetStockFont( STOCK_FONT _font );
		SwfRuntimeFont* LoadSpecialFont( std::string _path );

	};
} /* Swf */


#endif /* _SWFRUNTIMEBUILDER_H_ */
