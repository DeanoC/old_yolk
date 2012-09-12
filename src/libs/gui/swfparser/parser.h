// 
//  parser.h
//  parser
//  
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined( YOLK_GUI_SWFPARSER_PARSER_H_ )
#define YOLK_GUI_SWFPARSER_PARSER_H_

#include "SwfRect.h"
#include "SwfStream.h"
#include "SwfTag.h"
#include "SwfRGB.h"
#include "SwfDictionary.h"
#include "SwfFrame.h"

namespace Swf {
	// forward decl
	class SwfShapeObject;
	class SwfDisplayObject;
	class SwfRemoveObject;
	class SwfText;
	class SwfDynamicText;

	class Parser {
	public:
		virtual ~Parser(){}	
		void Parse( const std::string& _path );

		static const int MAX_VERSION_SUPPORTED = 7;
		SwfRect* headerRect;
		float frameRate;
		uint16_t frameCount;

		SwfStream stream;
		SwfDictionary dictionary;
		std::vector<SwfFrame*>* frameList;
		SwfFrame* currentFrame;
		bool hasBackgroundColour;
		SwfRGBA backgroundColour;
		int fileVersion;
		
	protected:
		int fileSize;
		struct JpegReader* jpegReader;
		void ParseHeader(std::string _path);
		bool ParseTag(bool _isSpriteTag);

		//        JpegDecoder tableDecoder;

		struct TagHandler {
			typedef void (Parser::*Handler)(int _length);
			TagHandler() {
				handler = NULL;
				spriteAllowed = false;
			}
			TagHandler(Handler _handler) {
				handler = _handler;
				spriteAllowed = false;
			}
			TagHandler(Handler _handler, bool _spriteAllowed) {
				handler = _handler;
				spriteAllowed = _spriteAllowed;
			}
			Handler handler;
			bool spriteAllowed;
		};

		TagHandler handlers[256];

		virtual void ProcessTag(SwfTag _tag, int _length);
		virtual void ProcessSpriteTag(SwfTag _tag, int _length);

		// RAW handlers
		void ProcessRawSetBackgroundColor(int _length);
		void ProcessRawDefineFont(int _length);
		void ProcessRawDefineFont2(int _length);
		void ProcessRawDefineShape(int _length);
		void ProcessRawDefineShape2(int _length);
		void ProcessRawDefineShape3(int _length);
		void ProcessRawPlaceObject(int _length);
		void ProcessRawPlaceObject2(int _length);
		void ProcessRawRemoveObject(int _length);
		void ProcessRawRemoveObject2(int _length);
		void ProcessRawShowFrame(int _length);
		void ProcessRawDefineText(int _length);
		void ProcessRawDefineText2(int _length);
		void ProcessRawDefineEditText(int _length);
		void ProcessRawDefineSprite(int _length);
		void ProcessRawDefineMorphShape(int _length);
		void ProcessRawDefineMorphShape2(int _length);
		void ProcessRawDoAction(int _length);
		void ProcessRawDefineBits(int _length);
		void ProcessRawJPEGTables(int _length);
		void ProcessRawDefineBitsJPEG2(int _length);
		void ProcessRawDefineBitsJPEG3(int _length);
		void ProcessRawDefineBitsLossless(int _length);
		void ProcessRawDefineBitsLossless2(int _length);
		void ProcessRawDefineBitsLossless(bool v2, int _length);
		void ProcessRawDefineButton(int _length);

		// Processed handlers
		virtual void ProcessHeader(SwfRect* _rect, float _frameRate, uint16_t _frameCount){ }
		virtual void ProcessSetBackgroundColor(SwfRGBA _backgroundColour) { }
		virtual void ProcessDefineShape(SwfShapeObject* _object){}
		virtual void ProcessDefineFont(SwfFont* _font) { }
		virtual void ProcessDisplayObject(SwfDisplayObject* _obj) { }
		virtual void ProcessText(SwfText* _text) { }
		virtual void ProcessDynamicText(SwfDynamicText* _dtext) { }
		virtual void ProcessRemoveObject(SwfRemoveObject* _obj) { }
	};
}
#endif