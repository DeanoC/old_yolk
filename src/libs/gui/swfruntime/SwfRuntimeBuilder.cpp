// 
//  SwfRuntimeBuilder.cpp
//  Projects
//  
//  Created by Deano on 2008-09-30.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#include "swfruntime.h"
#include "gui/SwfParser/parser.h"
#include "gui/SwfParser/SwfShape.h"
#include "gui/SwfParser/SwfRGB.h"
#include "gui/SwfParser/SwfFillStyle.h"
#include "gui/SwfParser/SwfParser.h"
#include "gui/SwfParser/SwfBitmap.h"
#include "gui/SwfParser/SwfFont.h"
#include "SwfRuntimeFillStyle.h"
#include "SwfBasePath.h"
#include "SwfGpuPath.h"
#include "SwfGpuLinePath.h"
#include "SwfRuntimeShape.h"
#include "SwfRuntimeShape.h"
#include "SwfRuntimeUtils.h"
#include "SwfRuntimeBitmap.h"
#include "SwfRuntimeFont.h"
#include "SwfRuntimeText.h"
#include "SwfRuntimeDynamicText.h"
#include "SwfPlayer.h"
#include "SwfRuntimeBuilder.h"

namespace Swf {
	void SwfRuntimeBuilder::BuildBitmaps(RuntimeBitmaps& _runtimeBitmaps) {
		// build paths from characters 
		for( 	std::map<uint16_t, SwfBitmap*>::const_iterator i = parser->dictionary.bitmaps.begin();
			i != parser->dictionary.bitmaps.end();
			++i ) {
			SwfBitmap* bitmap = (*i).second;
			_runtimeBitmaps[bitmap->id] = CORE_NEW SwfRuntimeBitmap(bitmap);
		}
    }

    void SwfRuntimeBuilder::BuildRuntimeShape(SwfRuntimeShape* _runShape, SwfShape* _swfShape) {
		if (_swfShape->fillStyleArray != NULL) {
			BuildFillStyles(_swfShape->fillStyleArray, _swfShape->numFillStyles, _runShape->fills);
			BuildLineStyles(_swfShape->lineStyleArray, false, _swfShape->numLineStyles, _runShape->lineFills);
		} else {
			// fake a simple red colour for now (fonts only?)
//			_runShape->fills.push_back( CORE_NEW iPhone::SwfRFSSolidColour(player, SwfRGBA(1.0f, 0.0f, 0.0f, 1.0f)) );
//			_runShape->lineFills.push_back( CORE_NEW iPhone::SwfRFSLine(player, NULL) );
		}

		BuildPath(_swfShape, _runShape->fills, _runShape->lineFills, _runShape->gpuPaths, _runShape->gpuLinePaths, false, false);
		GenerateShapeGpuData(_runShape);
    }

	SwfRuntimeFont* SwfRuntimeBuilder::LoadSpecialFont( std::string _path ) {
		Parser* fontParser = CORE_NEW Parser();
		fontParser->Parse(_path);

		// this should use 'special' swf that contain the font and dynamic text field
		// this produces glyphs for all western text that we can then subsitute when the actual
		// swf file asks for the built in one.
		assert(fontParser->dictionary.fonts.size() == 1);

		SwfFont* font = fontParser->dictionary.fonts[1]; // as we only have one font is should have id 1
		SwfRuntimeFont* rtFont = CORE_NEW SwfRuntimeFont(font);

		for( 	int i=0;i < font->numGlyphs;++i )
		{
			SwfShape* shape = font->shapes[i];
			SwfRuntimeShape* runtimeShape = CORE_NEW SwfRuntimeShape( shape );
			BuildRuntimeShape( runtimeShape, shape );
			rtFont->glyphShapes.push_back( runtimeShape );
		}
		return rtFont;
	}

	SwfRuntimeFont* SwfRuntimeBuilder::GetStockFont( STOCK_FONT _font ){
		switch( _font ){
			case SERIF:
//				return LoadSpecialFont( player->getStockPath() + "/_serif.swf" );
			case TYPEWRITER:
//				return LoadSpecialFont( player->getStockPath() + "/_typewriter.swf" );
			case SANS:
				return LoadSpecialFont( player->getStockPath() + "/_sans.swf" );
			default:
				return NULL;
		}
	}

	void SwfRuntimeBuilder::BuildFonts(RuntimeFonts& _runtimeFonts)
	{
		for( 	std::map<uint16_t, SwfFont*>::const_iterator i = parser->dictionary.fonts.begin();
				i != parser->dictionary.fonts.end();
				++i ) {
			SwfFont* font = (*i).second;
			// check for special system fonts, that flash files can use
			if( font->name == "_sans") {
				_runtimeFonts[font->id] = GetStockFont( SANS );
				continue;
			} else if( font->name == "_serif") {
				_runtimeFonts[font->id] = GetStockFont( SERIF );
				continue;
			}	else if( font->name == "_typewriter") {
				_runtimeFonts[font->id] = GetStockFont( TYPEWRITER );
				continue;
			}
			// if we have no shapes, substitue San Serif
			if (font->numGlyphs == 0) {
				_runtimeFonts[font->id] = GetStockFont( SANS );
				continue;
			}

			SwfRuntimeFont* rtFont = new SwfRuntimeFont(font);
			for( 	int i=0;i < font->numGlyphs;++i ) {
				SwfShape* shape = font->shapes[i];
				SwfRuntimeShape* runtimeShape = CORE_NEW SwfRuntimeShape( shape );
				BuildRuntimeShape( runtimeShape, shape );
				rtFont->glyphShapes.push_back( runtimeShape );
			}
			
			_runtimeFonts[font->id] = rtFont;			
		}
	}
	
    void SwfRuntimeBuilder::BuildCharacterPaths(CharacterPaths& _charPaths) {
		// build paths from characters 
		for( 	std::map<uint16_t, SwfCharacter*>::const_iterator i = parser->dictionary.characters.begin();
				i != parser->dictionary.characters.end();
				++i ) {
			SwfCharacter* character = (*i).second;

			if (character->type == CT_SHAPEOBJECT) {
				SwfShapeObject* shape = (SwfShapeObject*)character;
				SwfShape* swfShape = shape->shape;
				SwfRuntimeShape* runtimeShape = CORE_NEW SwfRuntimeShape(swfShape);
				BuildRuntimeShape(runtimeShape, swfShape);
				_charPaths[character->id] = runtimeShape;
			} else if (character->type == CT_SPRITE) {
				_charPaths[character->id] = NULL;
			} else if (character->type == CT_TEXT) {
				SwfText* text = (SwfText*)character;
				_charPaths[character->id] = CORE_NEW SwfRuntimeText(text);
			} else if (character->type == CT_DYNAMICTEXT) {
				SwfDynamicText* dtext = (SwfDynamicText*)character;
				_charPaths[character->id] = CORE_NEW SwfRuntimeDynamicText(dtext);
			}
/*        
            else if (character is SwfMorphShape)
            {
                SwfMorphShape mshape = character as SwfMorphShape;
                SwfRuntimeMorphShape runtimeMShape = CORE_NEW SwfRuntimeMorphShape();
                BuildRuntimeMorphShape(runtimeMShape, mshape);

                characterPaths.Add(character.id, runtimeMShape);
            }*/
        }
    }

    void SwfRuntimeBuilder::BuildFillStyles(SwfFillStyle**& _fills, int _numFills, std::vector<SwfRuntimeFillStyle*>& _rtfills ) {
			_rtfills.resize( _rtfills.size() + _numFills );
			std::vector<SwfRuntimeFillStyle*>& fills = _rtfills;

			for(int fillNum=0;fillNum < _numFills;++fillNum ) {
				SwfFillStyle* fillStyle = _fills[fillNum];
            switch (fillStyle->fillType) {
					case SolidFill:	{
						SwfSolidFillStyle* solid = (SwfSolidFillStyle*) fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSSolidColour(player, solid->colour);
						break;
					}
					case LinearGradientFill:
					case RadialGradientFill: {
						SwfGradientFillStyle* lgrad = (SwfGradientFillStyle*) fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSGradient(player, lgrad);
						break;
					}
					case RepeatingBitmapFill: {
						SwfBitmapFillStyle* bitrm = (SwfBitmapFillStyle*)fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSTexture(player, bitrm);
						break;
					}
					case ClippedBitmapFill:	{
						SwfBitmapFillStyle* bitcm = (SwfBitmapFillStyle*)fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSTexture(player, bitcm);
						break;
					}
					case NonSmoothedRepeatingBitmapFill: {
						SwfBitmapFillStyle* bitnsrm = (SwfBitmapFillStyle*)fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSTexture(player, bitnsrm);
						break;
					}
					case NonSmoothedClippedBitmapFill: {
						SwfBitmapFillStyle* bitnscm = (SwfBitmapFillStyle*)fillStyle;
//						fills[fillNum] = CORE_NEW iPhone::SwfRFSTexture(player, bitnscm);
						break;
					}
/*                case FillType.MorphSolidFill:
                    SwfMorphSolidFillStyle msfs = fillStyle as SwfMorphSolidFillStyle;
                    fills[fillNum] = CORE_NEW SwfRuntimeMorphSolidFillStyle(player, msfs.startColour, msfs.endColour);
                    break;*/
					case FocalRadialGradientFill:
					default: {
//                    fills[fillNum] = CORE_NEW iPhone::SwfRFSSolidColour(player, SwfRGBA(1.0f,0.0f, 0.0f, 1.0f));
                    break;
					}
            }
        }
    }

    void SwfRuntimeBuilder::BuildLineStyles(SwfLineStyle**& _lineFills, bool _morph, int _numFills, std::vector<SwfRuntimeFillStyle*>& _rtfills) {
		_rtfills.resize( _rtfills.size() + _numFills );
		std::vector<SwfRuntimeFillStyle*>& lineFills = _rtfills;
		for(int lineFillNum=0;lineFillNum < _numFills;++lineFillNum ) {
			SwfLineStyle* lineStyle = _lineFills[lineFillNum];

			if (_morph) {
				//                lineFills[lineFillNum++] = CORE_NEW SwfRuntimeMorphLineFillStyle(player, lineStyle as SwfMorphLineStyle);
			} else {
//				lineFills[lineFillNum] = CORE_NEW iPhone::SwfRFSLine(player, lineStyle);
			}
		}
    }

    void SwfRuntimeBuilder::GenerateShapeGpuData(SwfRuntimeShape* _runShape)
    {
		for(	std::vector<SwfBasePath*>::iterator i = _runShape->gpuPaths.begin();
				i != _runShape->gpuPaths.end();
				++i ) {
			SwfBasePath* path = *i;
			if (path != NULL) {
				path->GenerateGPUData();
			}
		}
		for(	std::vector<SwfBasePath*>::iterator i = _runShape->gpuLinePaths.begin();
				i != _runShape->gpuLinePaths.end();
				++i ) {
			SwfBasePath* path = *i;
			if (path != NULL) {
				path->GenerateGPUData();
			}
		}
    }

	void SwfRuntimeBuilder::BuildPath( const SwfShape* _swfShape, 
                    std::vector<SwfRuntimeFillStyle*>& _fillStyles, 
                    std::vector<SwfRuntimeFillStyle*>& _lineFillStyles,
                    std::vector<SwfBasePath*>& _fillPaths,
                    std::vector<SwfBasePath*>& _linePaths,
                    bool _morphShape,
                    bool _morphEnd ) {
		int fill0 = 0, fill1 = 0, lineStyle = 0;
		SwfVec2Twip lastPos(0, 0); // shape origin?
		int fillPathsOffset = 0;
		int linePathsOffset = 0;
		SwfRuntimeFillStyle* fillStyle0 = NULL;
		SwfRuntimeFillStyle* fillStyle1 = NULL;
		SwfRuntimeFillStyle* lineFillStyle = NULL;

		// morphs always use direct quadratic rendering rather than the subdiving renderer
		bool useDirectQuadratic = false; // TODO player.UseGpuDirectQuadraticDrawer | _morphShape;

		int fillLen = _fillStyles.size();
		int lineLen = _lineFillStyles.size();
		
		if( !useDirectQuadratic ){
			_fillPaths.resize( fillLen );
			_linePaths.resize( lineLen );
		}

		// morph ends never set the styles, this does so if we have any fills
		// we don't actually use the styles at this point we just need to edge
		// data
		if (_morphEnd) {
			fill0 = 1;
			fill1 = 0;
			lineStyle = 0;
		}

		for(int i=0;i < _swfShape->numShapeRecords;++i){			
			SwfShapeRecord* sr = _swfShape->shapeRecords[i];
			
			switch (sr->ShapeType()) {
				case SwfShapeRecord::End:
					break;
				case SwfShapeRecord::StyleChange: {
					SwfStyleChangeShapeRecord* sc = (SwfStyleChangeShapeRecord*) sr;

					if (sc->moveToFlag) {
						lastPos = sc->moveDelta;
					}
					if (sc->newStylesFlag) {
						// not allowed in side a morph shape
						assert(_morphShape == false);
						fillPathsOffset = fillLen;
						linePathsOffset = lineLen;
						BuildFillStyles(sc->fillStyleArray, sc->numFillStyles, _fillStyles);
						BuildLineStyles(sc->lineStyleArray, false, sc->numLineStyles, _lineFillStyles);
						_fillPaths.resize( _fillStyles.size() + _fillPaths.size() );
						_linePaths.resize( _lineFillStyles.size() + _linePaths.size() );
					}
					if (sc->fillStyle0Flag) {
						fill0 = sc->fillStyle0;
					}
					if (sc->fillStyle1Flag) {
						fill1 = sc->fillStyle1;
					}

					if (sc->lineStyleFlag) {
						lineStyle = sc->lineStyle;
					}

					// update any style changes
					if (fill0 != 0)
						fillStyle0 = _fillStyles[fill0 - 1];
					else
						fillStyle0 = NULL;

					if (fill1 != 0)
						fillStyle1 = _fillStyles[fill1 - 1];
					else
						fillStyle1 = NULL;

					if (lineStyle != 0)
						lineFillStyle = _lineFillStyles[lineStyle - 1];
					else
						lineFillStyle = NULL;

					if (fill0 > 0) {
						if (_fillPaths[fill0 - 1 + fillPathsOffset] == NULL) {
							assert(fillStyle0 != NULL);
	//						if (useDirectQuadratic)
	//							fillPaths[fill0 - 1 + fillPathsOffset] = new SwfGpuQuadraticPath(fillStyle0, lastPos);
	//						else
							_fillPaths[fill0 - 1 + fillPathsOffset] = CORE_NEW SwfGpuPath(fillStyle0);
							_fillPaths[fill0 - 1 + fillPathsOffset]->MoveTo(lastPos);
						}
						else if (sc->moveToFlag) {
							_fillPaths[fill0 - 1 + fillPathsOffset]->MoveTo(lastPos);
						}
					}
					if (fill1 > 0) {
						if (_fillPaths[fill1 - 1 + fillPathsOffset] == NULL) {
							assert(fillStyle1 != NULL);
	//						if (useDirectQuadratic)
	//							fillPaths[fill1 - 1 + fillPathsOffset] = new SwfGpuQuadraticPath(fillStyle1, lastPos);
	//						else
							_fillPaths[fill1 - 1 + fillPathsOffset] = CORE_NEW SwfGpuPath(fillStyle1);
							_fillPaths[fill1 - 1 + fillPathsOffset]->MoveTo(lastPos);
						} else if (sc->moveToFlag) {
							_fillPaths[fill1 - 1 + fillPathsOffset]->MoveTo(lastPos);
						}
					}
					if (lineStyle > 0) {
						if (_linePaths[lineStyle - 1 + linePathsOffset] == NULL) {
	//						if (useDirectQuadratic)
	//							linePaths[lineStyle - 1 + linePathsOffset] = new SwfGpuQuadraticLinePath(lineFillStyle, lastPos);
	//						else
								_linePaths[lineStyle - 1 + linePathsOffset] = CORE_NEW SwfGpuLinePath(lineFillStyle);
							_linePaths[lineStyle - 1 + linePathsOffset]->MoveTo(lastPos);
						} else if (sc->moveToFlag) {
							_linePaths[lineStyle - 1 + linePathsOffset]->MoveTo(lastPos);
						}
					}
					break;
				}
				case SwfShapeRecord::StraightEdge: {
					SwfStraightEdgeShapeRecord* se = (SwfStraightEdgeShapeRecord*) sr;
					if (fill0 > 0) {
						_fillPaths[fill0 - 1 + fillPathsOffset]->LineTo(lastPos, lastPos + se->delta);
					}
					if (fill1 > 0) {
						_fillPaths[fill1 - 1 + fillPathsOffset]->LineTo(lastPos + se->delta, lastPos);
					}
					if (lineStyle > 0) {
						_linePaths[lineStyle - 1 + linePathsOffset]->LineTo(lastPos, lastPos + se->delta);
					}
					lastPos = lastPos + se->delta;
					break;
				}
				case SwfShapeRecord::CurvedEdge: {
					SwfCurvedEdgeShapeRecord* sce = (SwfCurvedEdgeShapeRecord*) sr;
					if (fill0 > 0) {
						_fillPaths[fill0 - 1 + fillPathsOffset]->QuadraticBezier(lastPos,
						                                                        lastPos + sce->controlDelta,
						                                                        lastPos + sce->controlDelta + sce->anchorDelta);
					}
					if (fill1 > 0) {
						_fillPaths[fill1 - 1 + fillPathsOffset]->QuadraticBezier(lastPos + sce->controlDelta + sce->anchorDelta,
						                                                        lastPos + sce->controlDelta,
						                                                        lastPos);
					}
					if (lineStyle > 0) {
						_linePaths[lineStyle - 1 + linePathsOffset]->QuadraticBezier(lastPos,
						                                                        lastPos + sce->controlDelta,
						                                                        lastPos + sce->controlDelta + sce->anchorDelta);
					}
					lastPos = lastPos + sce->controlDelta + sce->anchorDelta;
					break;
				}
				default:
					break;
			}
		}
	}
    
} /* Swf */
