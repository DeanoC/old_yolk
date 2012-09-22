// 
//  Parser.cpp
//  Parser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 
#include "swfparser.h"

#include "parser.h"
#include "SwfRect.h"
#include "SwfStream.h"
#include "SwfTag.h"
#include "SwfRGB.h"
#include "SwfFont.h"
#include "SwfShape.h"
#include "SwfDisplayObject.h"
#include "SwfText.h"
#include "SwfMorphShape.h"
#include "SwfSprite.h"
#include "SwfActionByteCode.h"
#include "SwfBitmap.h"
#include "SwfImage.h"
#include "SwfButton.h"
namespace Swf {		
	void SwfParser::Parse( const std::string& _path ) {
		hasBackgroundColour = false;
		// misc handlers
		handlers[SetBackgroundColor] = TagHandler(&SwfParser::ProcessRawSetBackgroundColor);
		handlers[ShowFrame] = TagHandler(&SwfParser::ProcessRawShowFrame, true);
		handlers[DoAction] = TagHandler(&SwfParser::ProcessRawDoAction, true);

		// font handlers
		handlers[DefineFont] =  TagHandler(&SwfParser::ProcessRawDefineFont);
		handlers[DefineFont2] =  TagHandler(&SwfParser::ProcessRawDefineFont2);

		// shape handlers
		handlers[DefineShape] = TagHandler(&SwfParser::ProcessRawDefineShape);
		handlers[DefineShape2] = TagHandler(&SwfParser::ProcessRawDefineShape2);
		handlers[DefineShape3] = TagHandler(&SwfParser::ProcessRawDefineShape3);
		handlers[DefineMorphShape] = TagHandler(&SwfParser::ProcessRawDefineMorphShape);
		handlers[DefineMorphShape2] =TagHandler(&SwfParser::ProcessRawDefineMorphShape2);

		// sprite define handler
		handlers[DefineSprite] = TagHandler(&SwfParser::ProcessRawDefineSprite);

		// place object handlers
		handlers[PlaceObject] = TagHandler(&SwfParser::ProcessRawPlaceObject, true);
		handlers[PlaceObject2] = TagHandler(&SwfParser::ProcessRawPlaceObject2, true);
		handlers[RemoveObject] = TagHandler(&SwfParser::ProcessRawRemoveObject, true);
		handlers[RemoveObject2] = TagHandler(&SwfParser::ProcessRawRemoveObject2, true);

		// text object handlers
		handlers[DefineText] = TagHandler( &SwfParser::ProcessRawDefineText);
		handlers[DefineText2] = TagHandler( &SwfParser::ProcessRawDefineText2);
		handlers[DefineEditText] = TagHandler(&SwfParser::ProcessRawDefineEditText);

		// bitmap handlers
		handlers[DefineBits] = TagHandler(&SwfParser::ProcessRawDefineBits);
		handlers[JPEGTables] = TagHandler(&SwfParser::ProcessRawJPEGTables);
		handlers[DefineBitsJPEG2] = TagHandler(&SwfParser::ProcessRawDefineBitsJPEG2);
		handlers[DefineBitsJPEG3] = TagHandler(&SwfParser::ProcessRawDefineBitsJPEG3);
		handlers[DefineBitsLossless] = TagHandler(&SwfParser::ProcessRawDefineBitsLossless);
		handlers[DefineBitsLossless2] = TagHandler(&SwfParser::ProcessRawDefineBitsLossless2);
		
		// button handlers
		handlers[DefineButton] = TagHandler(&SwfParser::ProcessRawDefineButton);

		frameList = CORE_NEW std::vector<SwfFrame*>();
		currentFrame = CORE_NEW SwfFrame(); // initial frame, added on the show frame tag

		ParseHeader(_path);

		headerRect = SwfRect::Read(stream);
		frameRate = stream.readFixed8();
		frameCount = stream.readUInt16();

		ProcessHeader(headerRect, frameRate, frameCount);

		while ( ParseTag(false) ) {
			// keep parsing...
		}

	}
	
	bool SwfParser::ParseTag(bool _isSpriteTag) {
#if _DEBUG
		static uint16_t prevTagCodeAndLength = 0;
		static SwfTag prevTagCode;
		static SwfTag prevPrevTagCode;
		static uint64_t prevPrevMarker;
		static uint64_t prevMarker = 0;
		static bool gobackToPrev = false;
	redo:
		uint64_t debugMarker = stream.marker();
#endif
		uint16_t tagCodeAndLength = stream.readUInt16();
		SwfTag tagCode = (SwfTag)((tagCodeAndLength & ~0x3f) >> 6);
		assert((int)tagCode >=0 && (int)tagCode <= MAX_TAG );
		int length = (int)(tagCodeAndLength & 0x3f);
		if (length == 0x3f) {
			// long tag
			length = stream.readInt32();
		}
		uint64_t marker = stream.marker();
		
		if (_isSpriteTag) {
			ProcessSpriteTag(tagCode, length);
		} else {
			ProcessTag(tagCode, length);
		}
#if _DEBUG
		if(stream.marker() > (marker + length)) {
			gobackToPrev = false;
		}
		if( gobackToPrev ) {
			stream.setToMarker(prevPrevMarker);
			goto redo;			
		} else {
			prevPrevTagCode = prevTagCode;
			prevTagCode = tagCode;
			prevTagCodeAndLength = tagCodeAndLength;
			prevPrevMarker = prevMarker;
			prevMarker = debugMarker;
		}
#else
		assert(marker + length  >= stream.marker() );
#endif

		stream.setToMarker(marker + length);
		if (tagCode == End)
			return false;
		else 
			return true;
	}
	
	void SwfParser::ProcessTag(SwfTag _tag, int _length) {
		TagHandler::Handler handler = handlers[_tag].handler;
		if (handler != NULL)
			(this->*handler)(_length);
	}

	void SwfParser::ProcessSpriteTag(SwfTag _tag, int _length) {
		TagHandler::Handler handler = (handlers[_tag].spriteAllowed == false) ? NULL : handlers[_tag].handler;
		if (handler != NULL)
			(this->*handler)(_length);
	}
	//-----------------------------
	//-----------------------------
	// RAW Handlers (Convert bit stream into objects)
	//-----------------------------
	//------------------------------------

	void SwfParser::ProcessRawDefineFont(int _length) {
		SwfFont* font = SwfFont::Read(stream, 1);
		dictionary.fonts[font->id] = font;
		ProcessDefineFont(font);
	}
	
	void SwfParser::ProcessRawDefineFont2(int _length) {
		SwfFont* font = SwfFont::Read(stream, 2);
		dictionary.fonts[font->id] = font;
		ProcessDefineFont(font);
	}	
	
	void SwfParser::ProcessRawSetBackgroundColor(int _length) {
		backgroundColour = SwfRGBA::ReadRGB(stream);
		hasBackgroundColour = true;
		ProcessSetBackgroundColor(backgroundColour);
	}

	void SwfParser::ProcessRawDefineShape(int _length) {
		SwfShapeObject* shape = SwfShapeObject::Read(stream, 1);
		dictionary.characters[shape->id] = shape;
		ProcessDefineShape(shape);
	}
	
	void SwfParser::ProcessRawDefineShape2(int _length) {
		SwfShapeObject* shape = SwfShapeObject::Read(stream, 2);
		dictionary.characters[shape->id] = shape;
		ProcessDefineShape(shape);
	}
	
	void SwfParser::ProcessRawDefineShape3(int _length) {
		SwfShapeObject* shape = SwfShapeObject::Read(stream, 3);
		dictionary.characters[shape->id] = shape;
		ProcessDefineShape(shape);
	}

	void SwfParser::ProcessRawPlaceObject(int _length) {
		SwfDisplayObject* obj = SwfDisplayObject::Read(stream, 1);
		currentFrame->frameElements.push_back(obj);
		ProcessDisplayObject(obj);
	}
	
	void SwfParser::ProcessRawPlaceObject2(int _length) {
		SwfDisplayObject* obj = SwfDisplayObject::Read(stream,2);
		currentFrame->frameElements.push_back(obj);
		ProcessDisplayObject(obj);
	}
	
	void SwfParser::ProcessRawRemoveObject(int _length) {
		SwfRemoveObject* obj = SwfRemoveObject::Read(stream, 1);
		currentFrame->frameElements.push_back(obj);
		ProcessRemoveObject(obj);
	}
	
	void SwfParser::ProcessRawRemoveObject2(int _length) {
		SwfRemoveObject* obj = SwfRemoveObject::Read(stream,2);
		currentFrame->frameElements.push_back(obj);
		ProcessRemoveObject(obj);
	}

	void SwfParser::ProcessRawShowFrame(int _length) {
		frameList->push_back(currentFrame);
		currentFrame = CORE_NEW SwfFrame();
	}
	
	void SwfParser::ProcessRawDefineText(int _length) {
		SwfText* text = SwfText::Read(stream, 1);
		dictionary.characters[text->id] = text;
		ProcessText(text);
	}

	void SwfParser::ProcessRawDefineText2(int _length) {
		SwfText* text = SwfText::Read(stream, 2);
		dictionary.characters[text->id], text;
		ProcessText(text);
	}

	void SwfParser::ProcessRawDefineEditText(int _length) {
		SwfDynamicText* dtext = SwfDynamicText::Read(stream);
		dictionary.characters[dtext->id]  = dtext;
		ProcessDynamicText(dtext);
	}

	// Borrowed from GameSWF
	// Wrapper function -- uses Zlib to uncompress in_bytes worth
	// of data from the input file into buffer_bytes worth of data
	// into *buffer.
	void	inflate_wrapper(Core::InOutInterface* in, void* buffer, int buffer_bytes) {
		assert(in);
		assert(buffer);
		assert(buffer_bytes > 0);

		int err;
		z_stream d_stream; /* decompression stream */

		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;

		d_stream.next_in  = 0;
		d_stream.avail_in = 0;

		d_stream.next_out = (uint8_t*) buffer;
		d_stream.avail_out = (unsigned int) buffer_bytes;

		inflateInit(&d_stream);

		uint8_t	buf[1];

		for (;;) {
			// Fill a one-byte (!) buffer.
			buf[0] = in->getByte();
			d_stream.next_in = &buf[0];
			d_stream.avail_in = 1;

			err = inflate(&d_stream, Z_SYNC_FLUSH);
			if (err == Z_STREAM_END) break;
		}

		inflateEnd(&d_stream);
	}

	struct JpegSourceMgrWrapper {
		jpeg_source_mgr sourceMgr;
		Core::InOutInterface* io;
		bool start;
		int bufferSize;
		JOCTET *buffer;
		JpegSourceMgrWrapper(Core::InOutInterface* _io, int _bufferSize) {
			io = _io;
			buffer = CORE_NEW_ARRAY JOCTET[_bufferSize];
			bufferSize = _bufferSize;
			
			start = true;
			sourceMgr.init_source = init_source;
			sourceMgr.fill_input_buffer = fill_input_buffer;
			sourceMgr.skip_input_data = skip_input_data;
			sourceMgr.resync_to_restart = jpeg_resync_to_restart;	// use default method
			sourceMgr.term_source = term_source;
			sourceMgr.bytes_in_buffer = 0;
			sourceMgr.next_input_byte = NULL;
		}
		
		~JpegSourceMgrWrapper() {
			CORE_DELETE_ARRAY buffer;
			bufferSize = 0;
		}
		static void init_source(j_decompress_ptr cinfo) {
			JpegSourceMgrWrapper*	src = (JpegSourceMgrWrapper*) cinfo->src;
			src->start = true;
		}
		
		// Read data into our input buffer.  Client calls this
		// when it needs more data from the file.
		static boolean	fill_input_buffer(j_decompress_ptr cinfo) {
			JpegSourceMgrWrapper*	src = (JpegSourceMgrWrapper*) cinfo->src;

			uint64_t bytes_read = src->io->read(src->buffer, src->bufferSize);

			if (bytes_read <= 0) {
				// Is the file completely empty?
				if (src->start) {
					return FALSE;
				}
				// warn("jpeg end-of-stream");

				// Insert a fake EOI readInt.
				src->buffer[0] = (JOCTET) 0xFF;
				src->buffer[1] = (JOCTET) JPEG_EOI;
				bytes_read = 2;
			}

			// Hack to work around SWF bug: sometimes data
			// starts with FFD9FFD8, when it should be
			// FFD8FFD9!
			if (src->start && bytes_read >= 4)
			{
				if (src->buffer[0] == 0xFF
				    && src->buffer[1] == 0xD9 
				    && src->buffer[2] == 0xFF
				    && src->buffer[3] == 0xD8)
				{
					src->buffer[1] = 0xD8;
					src->buffer[3] = 0xD9;
				}
			}

			// Expose buffer state to clients.
			src->sourceMgr.next_input_byte = src->buffer;
			src->sourceMgr.bytes_in_buffer = (size_t) bytes_read;
			src->start = false;

			return TRUE;
		}

		static void	skip_input_data(j_decompress_ptr cinfo, long num_bytes)
		// Called by client when it wants to advance past some
		// uninteresting data.
		{
			JpegSourceMgrWrapper*	src = (JpegSourceMgrWrapper*) cinfo->src;

			// According to jpeg docs, large skips are
			// infrequent.  So let's just do it the simple
			// way.
			if (num_bytes > 0) {
				while (num_bytes > (long) src->sourceMgr.bytes_in_buffer) {
					num_bytes -= (long) src->sourceMgr.bytes_in_buffer;
					fill_input_buffer(cinfo);
				}
				// Handle remainder.
				src->sourceMgr.next_input_byte += (size_t) num_bytes;
				src->sourceMgr.bytes_in_buffer -= (size_t) num_bytes;
			}
		}

		// Terminate the source.  Make sure we get deleted.
		static void term_source(j_decompress_ptr cinfo) {
		}		
		void	discard_partial_buffer() {
			// Discard existing bytes in our buffer.
			sourceMgr.bytes_in_buffer = 0;
			sourceMgr.next_input_byte = NULL;
		}
		
		void ResizeBuffer( int _bufferSize) {
			if(bufferSize != 0){
				CORE_DELETE_ARRAY buffer;
				bufferSize = 0;
			}
			if(_bufferSize != 0){
				buffer = CORE_NEW_ARRAY JOCTET[_bufferSize];
				bufferSize = _bufferSize;
			}
		}
		
	};
	
	struct JpegReader {
		jpeg_decompress_struct	dinfo;
		jpeg_error_mgr	err;
		JpegSourceMgrWrapper* src;
		bool started;
		JpegReader( Core::InOutInterface* _io, int _bufferSize) {
			src = CORE_NEW JpegSourceMgrWrapper(_io, _bufferSize);
			jpeg_std_error(&err);
			
			dinfo.err = &err;
			jpeg_create_decompress(&dinfo);
			dinfo.src = (jpeg_source_mgr*)src;
			started = false;
		}
		~JpegReader() {
			EndImage();
			jpeg_destroy_decompress(&dinfo);
			CORE_DELETE( src );
		}
		// Discard any data sitting in our input buffer.  Use
		// this before/after reading headers or partial image
		// data, to avoid screwing up future reads.
		void	DiscardPartialBuffer() {
			JpegSourceMgrWrapper* src = (JpegSourceMgrWrapper*) dinfo.src;

			// We only have to discard the input buffer after reading the tables.
			if (src) {
				src->discard_partial_buffer();
			}
		}
		void StartImage() {
			assert(started == false);
			while(dinfo.global_state != 202) {
				jpeg_read_header(&dinfo, FALSE);
			}
			jpeg_start_decompress(&dinfo);
			started = true;
		}
		void EndImage() {
			if(started) {
				jpeg_finish_decompress(&dinfo);
				started = false;
			}
		}
		void ReadHeaderOnly() {
			jpeg_read_header(&dinfo, FALSE);
		}
		void ResizeBuffer( int _bufferSize) {
			src->ResizeBuffer(_bufferSize);
		}
		
		// Return the height of the image.  Take the data from our m_cinfo struct.
		int	GetHeight() const {
			assert(started);
			return dinfo.output_height;
		}

		// Return the width of the image.  Take the data from our m_cinfo struct.
		int	GetWidth() const {
			assert(started);
			return dinfo.output_width;
		}

		// Return number of components (i.e. == 3 for RGB
		// data).  The size of the data for a scanline is
		// get_width() * get_components().
		int	GetComponents() const {
			assert(started);
			return dinfo.output_components;
		}


		// Read a scanline's worth of image data into the
		// given buffer.  The amount of data read is
		// get_width() * get_components().
		void ReadScanline(uint8_t* rgb_data) {
			assert(started);
			assert(dinfo.output_scanline < dinfo.output_height);
			int	lines_read = jpeg_read_scanlines(&dinfo, &rgb_data, 1);
			assert(lines_read == 1);
			lines_read = lines_read;	// avoid warning in NDEBUG
		}
	};

	void SwfParser::ProcessRawDefineBits(int _length) {
		uint16_t id = stream.readUInt16();
		_length -= sizeof(uint16_t);
		SwfBitmap* bitmap = CORE_NEW SwfBitmap(id);

		if(jpegReader == NULL) {
			jpegReader = CORE_NEW JpegReader(stream.stream, _length);
		} else {
			jpegReader->DiscardPartialBuffer();
			jpegReader->ResizeBuffer(_length);
		}

		jpegReader->StartImage();
		int width = jpegReader->GetWidth();
		int height = jpegReader->GetHeight();
		int comps = jpegReader->GetComponents();
		uint8_t* buffer = CORE_NEW_ARRAY uint8_t[ width * height * comps];
		for(int i=0;i < height;++i) {
			jpegReader->ReadScanline(buffer + (i*width*comps));
		}
		jpegReader->EndImage();
		bitmap->width = width;
		bitmap->height = height;
		bitmap->image = CORE_NEW SwfRGBAImage(width,height,true,3,width*comps,buffer);
		CORE_DELETE_ARRAY buffer;
		dictionary.bitmaps[ bitmap->id ] = bitmap;
	}

	void SwfParser::ProcessRawJPEGTables(int _length) {
		assert(jpegReader == NULL);
		jpegReader = CORE_NEW JpegReader(stream.stream, _length);
		jpegReader->ReadHeaderOnly();
	}

	void SwfParser::ProcessRawDefineBitsJPEG2(int _length) {
		uint16_t id = stream.readUInt16();
		_length -= sizeof(uint16_t);
		SwfBitmap* bitmap = CORE_NEW SwfBitmap(id);

		// jpeg data
		jpegReader = CORE_NEW JpegReader(stream.stream, _length);
		jpegReader->StartImage();
		int width = jpegReader->GetWidth();
		int height = jpegReader->GetHeight();
		int comps = jpegReader->GetComponents();
		uint8_t* buffer = CORE_NEW_ARRAY uint8_t[ width * height * comps];
		for(int i=0;i < height;++i) {
			jpegReader->ReadScanline(buffer + (i*width*comps));
		}
		jpegReader->EndImage();
		bitmap->width = width;
		bitmap->height = height;
		bitmap->image = CORE_NEW SwfRGBAImage(width,height,true,3,width*comps,buffer);
		CORE_DELETE_ARRAY buffer;
		dictionary.bitmaps[bitmap->id] = bitmap;
	}

	void SwfParser::ProcessRawDefineBitsJPEG3(int _length) {
		uint16_t id = stream.readUInt16();
		uint32_t jpegSize = stream.readUInt32();

		// jpeg data
		jpegReader = CORE_NEW JpegReader(stream.stream, jpegSize);
		jpegReader->StartImage();
		int width = jpegReader->GetWidth();
		int height = jpegReader->GetHeight();
		int comps = jpegReader->GetComponents();
		uint8_t* buffer = CORE_NEW_ARRAY uint8_t[ width * height * comps ];
		for(int i=0;i < height;++i) {
			jpegReader->ReadScanline(buffer + (i*width*comps));
		}
		jpegReader->EndImage();
		
		// seperate alpha data
		uint8_t* data = CORE_NEW_ARRAY uint8_t[width * height];
//		inflate_wrapper(stream.stream,data, _length - jpegSize );

		// TODO merge alpha stream in with RGB
		SwfBitmap* bitmap = CORE_NEW SwfBitmap(id);
		bitmap->width = width;
		bitmap->height = height;		
		bitmap->image = CORE_NEW SwfRGBAImage(width,height,true,3,width*comps,buffer);
		CORE_DELETE_ARRAY buffer;

		CORE_DELETE_ARRAY data;
		dictionary.bitmaps[bitmap->id] = bitmap;
	}

	void SwfParser::ProcessRawDefineBitsLossless(int _length) {
		ProcessRawDefineBitsLossless(false, _length);
	}

	void SwfParser::ProcessRawDefineBitsLossless2(int _length) {
		ProcessRawDefineBitsLossless(true, _length);
	}

	void SwfParser::ProcessRawDefineBitsLossless(bool v2, int _length) {
		uint16_t id = stream.readUInt16();
		_length -= sizeof(uint16_t);
		SwfBitmap* bitmap = CORE_NEW SwfBitmap(id);

		uint8_t format = stream.readUInt8();
		uint16_t width = stream.readUInt16();
		uint16_t height = stream.readUInt16();

		bitmap->width = width;
		bitmap->height = height;

		int colourTableSize = 0;
		int decompSize = 0;
		if (format == 0x3) {
			colourTableSize = stream.readUInt8() + 1;
			int dataSize = (colourTableSize * 3) + (Core::alignTo(width, 4) * height);
			uint8_t* data = CORE_NEW_ARRAY uint8_t[dataSize];
			inflate_wrapper(stream.stream,data,dataSize);

			uint8_t* clut = data;
			uint8_t* image = data + (colourTableSize * 3);
			bitmap->image = CORE_NEW SwfClutImage(width, height, !v2, colourTableSize, clut, image);
			CORE_DELETE_ARRAY data;
		} else if (format == 0x4) {
			// in theory not supported for Lossless 2 but no harm leaving it here...
			decompSize = (Core::alignTo(width * 2, 4) * height);
			uint8_t* decompBuf = CORE_NEW_ARRAY uint8_t[decompSize];
			inflate_wrapper(stream.stream, decompBuf, decompSize);
			bitmap->image = CORE_NEW Swf1555Image(width, height, Core::alignTo(width * 2, 4), decompBuf);
			CORE_DELETE_ARRAY decompBuf;
		} else if (format == 0x5) {
			decompSize = (Core::alignTo(width * 4, 4) * height);
			uint8_t* decompBuf = CORE_NEW_ARRAY uint8_t[decompSize];
			inflate_wrapper(stream.stream, decompBuf, decompSize);
			bitmap->image = CORE_NEW SwfRGBAImage(width, height, !v2, 4, Core::alignTo(width * 4, 4), decompBuf);
			CORE_DELETE_ARRAY decompBuf;
		}
		dictionary.bitmaps[bitmap->id] = bitmap;
	}

	/// <summary>
	/// Defining Sprite is special, as is a hierachy of tags
	/// and as such needs closer interaction with the parser than 
	/// other tags
	/// </summary>
	void SwfParser::ProcessRawDefineSprite(int _length) {
		// back up the main frame list
		std::vector<SwfFrame*>* backup = frameList;
		SwfFrame* backupCurrent = currentFrame;

		uint16_t id = stream.readUInt16();
		SwfSprite* sprite = CORE_NEW SwfSprite(id);
		// sprites need a full parser as they contain frames and tags
		sprite->frameCount = stream.readUInt16();
		// stick in this sprites one
		frameList = &sprite->frames;
		currentFrame = CORE_NEW SwfFrame();

		while (ParseTag(true)) {
			// keep parsing...
		}

		// we processed all teh frames, now pop
		// the main frame list back were it belongs
		frameList = backup;
		currentFrame = backupCurrent;
		dictionary.characters[sprite->id] = sprite;
	}

	void SwfParser::ProcessRawDefineMorphShape(int _length) {
		SwfMorphShape* morph = SwfMorphShape::Read(stream, 1);
		dictionary.characters[morph->id] = morph;
	}
	
	void SwfParser::ProcessRawDefineMorphShape2(int _length) {
		SwfMorphShape* morph = SwfMorphShape::Read(stream, 2);
		dictionary.characters[morph->id] = morph;
	}

	void SwfParser::ProcessRawDoAction(int _length) {
		SwfActionByteCode* actionScript = CORE_NEW SwfActionByteCode();
		actionScript->byteCode = CORE_NEW_ARRAY uint8_t[_length];
		actionScript->lengthInBytes = _length;
		if( fileVersion < 7 ) {
			actionScript->isCaseSensitive = false;
		} else {
			actionScript->isCaseSensitive = true;
		}
		stream.readBytes(actionScript->byteCode, actionScript->lengthInBytes);
		currentFrame->frameElements.push_back(actionScript);
	}

	void SwfParser::ProcessRawDefineButton(int _length) {
//		SwfMorphShape* morph = SwfMorphShape::Read(stream, 2);
//		dictionary.characters[morph->id] = morph;
	}

	//-----------------------------
	void SwfParser::ParseHeader(std::string _path) {
		Core::File* osstream = CORE_NEW Core::File( _path.c_str() );
		assert(osstream != NULL && osstream->isValid());

		SwfStream& swfStream = stream;
		bool compressed = false;

		// SWF are little endian
		EndianBitConverter endianConverter = EndianBitConverter::CreateForLittleEndian();

		// signature0 = F | C 
		// signature1 = W 
		// signature2 = S
		// version = 0x1 to 0x9
		uint8_t signature[3];
		uint8_t version;
		osstream->read( signature, 3);

		if ((signature[0] == 'F' || signature[0] == 'C') ||
			(signature[1] == 'W') ||
			(signature[2] == 'S')) {
			if (signature[0] == 'C') {
				compressed = true;
			}
			version = osstream->getByte();

			fileVersion = version;

			// for uncompressed file this is the size on disk
			// for compressed files, this is size after decompression
			uint8_t array[4];
			osstream->read(array,4);

			fileSize = endianConverter.ToInt32(array);

			if( compressed == false ) {
				// at the mo, stream off disk, tho for performance may make sense to read into a memory stream?
				swfStream.setStream(osstream);
			} else {
				// at the moment decompress the entire file first
				// should use a decompression stream in future
				uint8_t* decompBuf = CORE_NEW_ARRAY uint8_t[fileSize];
				uint64_t bytesLeft = osstream->bytesLeft();
				uint8_t* compBuf = CORE_NEW_ARRAY uint8_t[(unsigned int) bytesLeft]; // > 32 bit not suppored on 32 bit OSs
				osstream->read(compBuf,bytesLeft);
				uLongf fs = fileSize;
				uncompress( decompBuf, &fs, compBuf, (uLong) bytesLeft ); // TODO uncompress doesn't support 64 bit on windows it seems
				CORE_DELETE( osstream );
				CORE_DELETE_ARRAY( compBuf );
				swfStream.setStream(CORE_NEW Core::MemFile(decompBuf, fs));
			}
		}
		else {
			assert(false);
		}

		swfStream.version = version;
    }
}