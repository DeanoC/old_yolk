// 
//  SwfTag.h
//  SwfParser
//  
//  Created by Deano on 2008-09-27.
//  Copyright 2012 Cloud Pixies Ltd. All rights reserved.
// 

#if !defined(_SWF_TAG_H)
#define _SWF_TAG_H

namespace Swf
{
	enum SwfTag
    {
        End = 0,
        ShowFrame = 1,
        DefineShape = 2,
        FreeCharacter = 3,
        PlaceObject = 4,
        RemoveObject = 5,
        DefineBits = 6,
        DefineButton = 7,
        JPEGTables = 8,
        SetBackgroundColor = 9,
        DefineFont = 10,
        DefineText = 11,
        DoAction = 12,
        DefineFontInfo = 13,
        // v 2.0
        DefineSound = 14,
        StartSound = 15,
        StopSound = 16,
        DefineButtonSound = 17,
        SoundStreamHead = 18,
        SoundStreamBlock = 19,
        DefineBitsLossless = 20,
        DefineBitsJPEG2 = 21,
        DefineShape2 = 22,
        DefineButtonCxform = 23,
        Protect = 24,
        // v 3.0
        PathsArePostscript = 25,
        PlaceObject2 = 26,
        UnknownTag_27 = 27,
        RemoveObject2 = 28,
        SyncFrame = 29,
        UnknownTag_30 = 30,
        FreeAll = 31,
        DefineShape3 = 32,
        DefineText2 = 33,
        DefineButton2 = 34,
        DefineBitsJPEG3 = 35,
        DefineBitsLossless2 = 36,
        DefineEditText = 37, // v 4.0
        DefineVideo = 38, // v 4.0
        DefineSprite = 39,
        NameCharacter = 40,
        SerialNumber = 41,
        DefineTextFormat = 42,
        FrameLabel = 43,
        UnknownTag_44 = 44,
        SoundStreamHead2 = 45,
        DefineMorphShape = 46,
        GenerateFrame = 47,
        DefineFont2 = 48,
        GeneratorCommand = 49,
        // v 5.0
        DefineCommandObject = 50,
        CharacterSet = 51,
        ExternalFont = 52,
        UnknownTag_53 = 53,
        UnknownTag_54 = 54,
        UnknownTag_55 = 55,
        ExportAssets = 56,
        ImportAssets = 57,
        EnableDebugger = 58,
        // v 6.0
        DoInitAction = 59,
        DefineVideoStream = 60,
        VideoFrame = 61,
        DefineFontInfo2 = 62,
        UnknownTag_63 = 63,
        EnableDebugger2 = 64,
        // v 7.0
        ScriptLimits = 65,
        SetTabIndex = 66,
        UnknownTag_67 = 67,
        UnknownTag_68 = 68,
        // v 8.0
        FileAttributes = 69,
        PlaceObject3 = 70,
        ImportAssets2 = 71,
        UnknownTag_72 = 72,
        DefineFontalignZones = 73,
        CSMTextSettings = 74,
        DefineFont3 = 75,
        UnknownTag_76 = 76,
        MetaData = 77,
        DefineScalingGrid = 78,
        UnknownTag_79 = 79,
        UnknownTag_80 = 80,
        UnknownTag_81 = 81,
        UnknownTag_82 = 82,
        DefineShape4 = 83,
        DefineMorphShape2 = 84,		

		MAX_TAG = 85
    };
}

#endif
