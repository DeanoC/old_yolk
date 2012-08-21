/** \file LWOLoader.cpp
   Light wave 7 Object Loader implementation.
   A generic template C++ files
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "lightwaveloader.h"
#include "core/exception.h"
#include "LWOLoader.h"
#include <algorithm>

using namespace LightWave;

#if !defined( UNREFERENCED_PARAMETER )
#define UNREFERENCED_PARAMETER(x) (void)x
#endif
namespace
{
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------
#define CHUNK_NAME(a,b,c,d) ( ((d)<<24) | ((c)<<16) | ((b)<<8) | ((a)<<0) )

enum CHUNK_ENUM
{
	CHUNK_FORM = CHUNK_NAME('F','O','R','M'),
	CHUNK_LWOB = CHUNK_NAME('L','W','O','B'),
	CHUNK_PNTS = CHUNK_NAME('P','N','T','S'),
	CHUNK_SRFS = CHUNK_NAME('S','R','F','S'),
	CHUNK_POLS = CHUNK_NAME('P','O','L','S'),
	CHUNK_CRVS = CHUNK_NAME('C','R','V','S'),
	CHUNK_PCHS = CHUNK_NAME('P','C','H','S'),
	CHUNK_SURF = CHUNK_NAME('S','U','R','F'),
// V2 chunks
	CHUNK_TAGS = CHUNK_NAME('T','A','G','S'),
	CHUNK_LAYR = CHUNK_NAME('L','A','Y','R'),
	CHUNK_PTAG = CHUNK_NAME('P','T','A','G'),
		CHUNK_PART = CHUNK_NAME('P','A','R','T'),
		CHUNK_SMGP = CHUNK_NAME('S','M','G','P'),

	CHUNK_CLIP = CHUNK_NAME('C','L','I','P'),
	CHUNK_VMAP = CHUNK_NAME('V','M','A','P'),
	CHUNK_VMAD = CHUNK_NAME('V','M','A','D'),

// Clip sub chunks
		CHUNK_STIL = CHUNK_NAME('S','T','I','L'),
// Surface sub chunks
		CHUNK_COLR = CHUNK_NAME('C','O','L','R'),
		CHUNK_FLAG = CHUNK_NAME('F','L','A','G'),
		CHUNK_LUMI = CHUNK_NAME('L','U','M','I'),
		CHUNK_DIFF = CHUNK_NAME('D','I','F','F'),
		CHUNK_SPEC = CHUNK_NAME('S','P','E','C'),
		CHUNK_REFL = CHUNK_NAME('R','E','F','L'),
		CHUNK_TRAN = CHUNK_NAME('T','R','A','N'),
		CHUNK_TRNL = CHUNK_NAME('T','R','N','L'),
		CHUNK_GLOS = CHUNK_NAME('G','L','O','S'),
		CHUNK_BLOK = CHUNK_NAME('B','L','O','K'),

		CHUNK_SIDE = CHUNK_NAME('S','I','D','E'),

// Block sub chunks
		CHUNK_IMAP = CHUNK_NAME('I','M','A','P'),
		CHUNK_PROC = CHUNK_NAME('P','R','O','C'),
		CHUNK_GRAD = CHUNK_NAME('G','R','A','D'),
		CHUNK_SHDR = CHUNK_NAME('S','H','D','R'),

		CHUNK_CHAN = CHUNK_NAME('C','H','A','N'),
		CHUNK_ENAB = CHUNK_NAME('E','N','A','B'),
		CHUNK_OPAC = CHUNK_NAME('O','P','A','C'),
		CHUNK_AXIS = CHUNK_NAME('A','X','I','S'),
		CHUNK_TMAP = CHUNK_NAME('T','M','A','P'),
		CHUNK_PROJ = CHUNK_NAME('P','R','O','J'),
		CHUNK_CNTR = CHUNK_NAME('C','N','T','R'),
		CHUNK_SIZE = CHUNK_NAME('S','I','Z','E'),
		CHUNK_ROTA = CHUNK_NAME('R','O','T','A'),
		CHUNK_OREF = CHUNK_NAME('O','R','E','F'),
		CHUNK_FALL = CHUNK_NAME('F','A','L','L'),
		CHUNK_CSYS = CHUNK_NAME('C','S','Y','S'),
		CHUNK_IMAG = CHUNK_NAME('I','M','A','G'),

		CHUNK_VLUM = CHUNK_NAME('V','L','U','M'),
		CHUNK_VDIF = CHUNK_NAME('V','D','I','F'),
		CHUNK_VSPC = CHUNK_NAME('V','S','P','C'),
		CHUNK_VRFL = CHUNK_NAME('V','R','F','L'),
		CHUNK_VTRN = CHUNK_NAME('V','T','R','N'),

		CHUNK_RFLT = CHUNK_NAME('R','F','L','T'),
		CHUNK_RIMG = CHUNK_NAME('R','I','M','G'),
		CHUNK_RSAN = CHUNK_NAME('R','S','A','N'),
		CHUNK_RIND = CHUNK_NAME('R','I','N','D'),
		CHUNK_EDGE = CHUNK_NAME('E','D','G','E'),
		CHUNK_SMAN = CHUNK_NAME('S','M','A','N'),

		CHUNK_CTEX = CHUNK_NAME('C','T','E','X'),
		CHUNK_DTEX = CHUNK_NAME('D','T','E','X'),
		CHUNK_STEX = CHUNK_NAME('S','T','E','X'),
		CHUNK_RTEX = CHUNK_NAME('R','T','E','X'),
		CHUNK_TTEX = CHUNK_NAME('T','T','E','X'),
		CHUNK_LTEX = CHUNK_NAME('L','T','E','X'),
		CHUNK_BTEX = CHUNK_NAME('B','T','E','X'),

		CHUNK_TFLG = CHUNK_NAME('T','F','L','G'),
		CHUNK_TSIZ = CHUNK_NAME('T','S','I','Z'),
		CHUNK_TCTR = CHUNK_NAME('T','C','T','R'),
		CHUNK_TFAL = CHUNK_NAME('T','F','A','L'),
		CHUNK_TVEL = CHUNK_NAME('T','V','E','L'),

		CHUNK_TCLR = CHUNK_NAME('T','C','L','R'),
		CHUNK_TVAL = CHUNK_NAME('T','V','A','L'),
		CHUNK_TAMP = CHUNK_NAME('T','A','M','P'),

		CHUNK_TFP0 = CHUNK_NAME('T','F','P','0'),
		CHUNK_TFP1 = CHUNK_NAME('T','F','P','1'),
		CHUNK_TSP0 = CHUNK_NAME('T','S','P','0'),
		CHUNK_TFRQ = CHUNK_NAME('T','F','R','Q'),

		CHUNK_TIMG = CHUNK_NAME('T','I','M','G'),
		CHUNK_TALP = CHUNK_NAME('T','A','L','P'),

		CHUNK_TWRP = CHUNK_NAME('T','W','R','P'),
		CHUNK_TAAS = CHUNK_NAME('T','A','A','S'),
		CHUNK_TOPC = CHUNK_NAME('T','O','P','C'),

		CHUNK_SDAT = CHUNK_NAME('S','D','A','T'),

		CHUNK_IMSQ = CHUNK_NAME('I','M','S','Q'),
		CHUNK_FLYR = CHUNK_NAME('F','L','Y','R'),
		CHUNK_IMCC = CHUNK_NAME('I','M','C','C'),


		CHUNK_BUMP = CHUNK_NAME('B','U','M','P'), // may just be an ID

		MAX_CHUNK_ENUM = 0x7FFFFFFF
};

enum CHUNK_WORD_TYPE
{
	CHUNK_WORD_LONG,
	CHUNK_WORD_SHORT
};

//---------------------------------------------------------------------------
// Local Classes and structures
//---------------------------------------------------------------------------

#pragma pack(1)

struct CHUNK_HEADER_LONG
{
    uint32_t	Id;
    uint32_t	Len;
};

struct CHUNK_HEADER_SHORT
{
    uint32_t	Id;
    uint16_t	Len;
};

#pragma pack()

struct CHUNK_DATA
{
	CHUNK_ENUM			Type;
	void (LightWave::LWO_Loader::*FuncV1)(FILE *f, long cur,long max);
	void (LightWave::LWO_Loader::*FuncV2)(FILE *f, long cur,long max);
};

// LWO chunks
CHUNK_DATA ChunkNames[] = {
	{CHUNK_FORM,   	&LightWave::LWO_Loader::FORMReader, &LightWave::LWO_Loader::FORMReader  },
	{CHUNK_PNTS,   	&LightWave::LWO_Loader::PNTSReader, &LightWave::LWO_Loader::PNTSReader2 },	// Points
	{CHUNK_SRFS,   	&LightWave::LWO_Loader::SRFSReader, &LightWave::LWO_Loader::SkipReader  },	// Surfaces
	{CHUNK_POLS,   	&LightWave::LWO_Loader::POLSReader, &LightWave::LWO_Loader::POLSReader2 },	// Polygons
	{CHUNK_CRVS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader  },	// Curves
	{CHUNK_PCHS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader  },	// Patches
 	{CHUNK_SURF,   	&LightWave::LWO_Loader::SURFReader, &LightWave::LWO_Loader::SURFReader2  },	// Surface

	// V2 Chunks
	{CHUNK_TAGS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::TAGSReader2 },	// Tags name
	{CHUNK_LAYR,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::LAYRReader2 },	// Layer
	{CHUNK_PTAG,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::PTAGReader2 },  // Polygon Tags
	{CHUNK_CLIP,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::CLIPReader2 },  // Clip chunk
	{CHUNK_VMAP,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::VMAPReader2 },	// VMAP chunk
	{CHUNK_VMAD,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::VMADReader2 },	// VMAD chunk
};

// LWO sub chunks
CHUNK_DATA SubChunkNames[] = {
	// CLIP sub chunk
	{CHUNK_STIL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::STILReader2 }, // bmp file

	// SURF sub chunk
	{CHUNK_COLR,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::COLRReader2 }, // Base colour
	{CHUNK_FLAG,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Surface flags
	{CHUNK_LUMI,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::LUMIReader2 },	// Luminosity
	{CHUNK_DIFF,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::DIFFReader2 },	// Diffuse
	{CHUNK_SPEC,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SPECReader2 },	// Specular
	{CHUNK_REFL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::REFLReader2 },	// Reflection
	{CHUNK_TRAN,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::TRANReader2 },	// Transparency
	{CHUNK_TRNL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::TRNLReader2 },	// Translucency
	{CHUNK_SIDE,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SIDEReader2 },	// Double sidedness
	{CHUNK_BLOK,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::BLOKReader2 },	// BLOK sub segment
	
	// BLOK sub chunks
	{CHUNK_IMAP,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::IMAPReader2 },	// IMAP sub segment
	{CHUNK_PROC,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// PROC sub segment
	{CHUNK_GRAD,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// GRAD sub segment
	{CHUNK_SHDR,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// SHDR sub segment

	{CHUNK_CHAN,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::CHANReader2 }, // CHAN sub segment
	{CHUNK_ENAB,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::ENABReader2 }, // ENAB sub segment
	{CHUNK_OPAC,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },  // OPAC sub segment
	{CHUNK_AXIS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::AXISReader2 }, // AXIS sub segment
	{CHUNK_TMAP,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::TMAPReader2 }, // TMAP sub segment
	{CHUNK_PROJ,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::PROJReader2 }, // PROJ sub segment
	{CHUNK_CNTR,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::CNTRReader2 }, // CNTR sub segment
	{CHUNK_SIZE,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SIZEReader2 }, // SIZE sub segment
	{CHUNK_ROTA,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::ROTAReader2 }, // ROTA sub segment
	{CHUNK_OREF,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader }, // OREF sub segment
	{CHUNK_FALL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader }, // FALL sub segment
	{CHUNK_CSYS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::CSYSReader2 }, // CSYS sub segment
	{CHUNK_IMAG,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::IMAGReader2 }, // CSYS sub segment

	{CHUNK_VLUM,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Luminosity
	{CHUNK_VDIF,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Diffuse
	{CHUNK_VSPC,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Specular
	{CHUNK_VRFL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Reflection
	{CHUNK_VTRN,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Transparency
	{CHUNK_GLOS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Glosiness
	{CHUNK_RFLT,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Reflection mode
	{CHUNK_RIMG,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Reflection map image
	{CHUNK_RSAN,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Reflection map Image seam angle
	{CHUNK_RIND,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Refractive index
	{CHUNK_EDGE,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Edge transparency threshold
	{CHUNK_SMAN,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Max smoothing angle
	{CHUNK_CTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_DTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_STEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_RTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_TTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_LTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_BTEX,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_TFLG,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture flags
	{CHUNK_TSIZ,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture map size
	{CHUNK_TCTR,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture map center
	{CHUNK_TFAL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture map falloff
	{CHUNK_TVEL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture map velocity
	{CHUNK_TCLR,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture colour
	{CHUNK_TVAL,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// 
	{CHUNK_TAMP,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// texture bump map amplitude
	{CHUNK_TFP0,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Algorithmic texture params
	{CHUNK_TFP1,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Algorithmic texture params
	{CHUNK_TSP0,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_TFRQ,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_TIMG,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Image map
	{CHUNK_TALP,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Image alpha
	{CHUNK_TWRP,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Image wrap options
	{CHUNK_TAAS,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Anti-aliasing strength
	{CHUNK_TOPC,   	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Texture opacity
	{CHUNK_SHDR,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Shader plugin
	{CHUNK_SDAT,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },	// Shader plugin data
	{CHUNK_IMSQ,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_FLYR,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_IMCC,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::SkipReader },
	{CHUNK_VMAP,	&LightWave::LWO_Loader::SkipReader, &LightWave::LWO_Loader::VMAPSubReader2 },
};

//---------------------------------------------------------------------------
// Local Prototypes & Functions
//---------------------------------------------------------------------------

/**
returns the chunk index if available.
scans the chunk list and return the index if found
@param Id chunk Identifier
@return index in chunk table or -1 if not found
*/
int findChunk(CHUNK_ENUM Id)
{ 
	int i;
	
	for (i = 0; i < sizeof(ChunkNames)/sizeof(ChunkNames[0]); i++)
	{
		if (Id == ChunkNames[i].Type)
			return i;
	}

	return -1;
}

/**
returns the chunk index if available.
scans the chunk list and return the index if found
@param Id chunk Identifier
@return index in chunk table or -1 if not found
*/
int findSubChunk(CHUNK_ENUM Id)
{ 
	int i;
	
	for (i = 0; i < sizeof(SubChunkNames)/sizeof(SubChunkNames[0]); i++)
	{
		if (Id == SubChunkNames[i].Type)
			return i;
	}

	return -1;
}

} // end local namespace

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------
/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
LWO_Loader::LWO_Loader( const std::string& filename ) :
	lwoVersion(1),
	curLayer(0),
	curClip(0),
	curSurface(0)
{
	FILE *f;
	int p;

	// check for null
	if( filename.empty() || filename == "Null" )
		return;
	// open the file
	f = fopen(filename.c_str(),"rb");
	if (f == NULL)
		CoreThrowException(FileNotFoundError,filename.c_str());

	// Find file size.
	fseek(f, 0, SEEK_END);
	p = ftell(f);
	fseek(f, 0, SEEK_SET);

	// Go!
	CoreTry
	{
		ChunkReader(f, 0, p);
	} 
	CoreCatch( Except& )
	{
		fclose(f);
		CoreReThrow;
	};

	fclose(f);

	SortSurfaceBlocks();
}

LWO_Loader::~LWO_Loader()
{
	std::map<unsigned int, Surface*>::iterator surfIt = surfaceMap.begin();
	while( surfIt != surfaceMap.end() )
	{
		delete surfIt->second;
		++surfIt;
	}
	surfaceMap.clear();
}

void LWO_Loader::SortSurfaceBlocks()
{
	std::map<unsigned int, Surface*>::iterator surfIt = surfaceMap.begin();
	while( surfIt != surfaceMap.end() )
	{
		Surface* curSur = surfIt->second;

		// go through this surfaces blocks assigning them to the correct channel
		std::vector<LWBlok>::iterator blokIt = curSur->blocks.begin();
		while( blokIt != curSur->blocks.end() )
		{
			switch( (*blokIt).channel )
			{
			case LWBlok::COLR:
				curSur->colourBlocks.push_back( &(*blokIt) );
				break;
			default:
				// not handled yet
				break;
			}
			++blokIt;
		}

		// sort them
		std::sort(	curSur->colourBlocks.begin(), 
					curSur->colourBlocks.end(),
					LWBlok::OrdinalSortFunctor );

		++surfIt;
	}

}

/**
returns the tag id if available.
scans the tag list and return the tagid if found
@param tag tag string
@return id in tag list or -1 if not found
*/
int LWO_Loader::findTagId(std::string& tag)
{
	std::vector<std::string>::iterator vIt = tags.begin();

	int count = 0;
	while( vIt != tags.end() )
	{
		if( *vIt == tag)
			return count;

		count++;
		vIt++;
	}

	return -1;
}

/**
reads the chunk.
from this file position, executes this patch
@param f file handle
@param posInFile current position
@param maxSize the end of this chunk
@return position afterwards
*/
long LWO_Loader::dispatchChunk(FILE *f, long posInFile,long maxSize)
{
	long posInFileNow;
	int n;

	UNREFERENCED_PARAMETER( maxSize );

	// read chunk name
	ID4 ChunkName = read(f, ChunkName );
	// read length of this data segment
	U4 Len = read(f, Len);

	n = findChunk( *((CHUNK_ENUM*) &ChunkName) );

	if(n < 0) {
		LOG(INFO) << "Unknown Chunk : "	<< ((unsigned char*)&ChunkName)[0] 
									<< ((unsigned char*)&ChunkName)[1] 
									<< ((unsigned char*)&ChunkName)[2] 
									<< ((unsigned char*)&ChunkName)[3] 
									<< "\n";
	} else
	{
		// replaced later but used for subchunks etc
		posInFileNow = posInFile + sizeof(CHUNK_HEADER_LONG);

		if(lwoVersion == 1) {
			if( ChunkNames[n].FuncV1 == &LWO_Loader::SkipReader)
				LOG(INFO) << "Skipping Chunk : " << *((unsigned int*)&ChunkName) << "\n";

			(this->*ChunkNames[n].FuncV1)(f, posInFileNow, Len);
		} else {
			if( ChunkNames[n].FuncV2 == &LWO_Loader::SkipReader)
				LOG(INFO) << "Skipping Chunk : " << *((unsigned int*)&ChunkName) << "\n";

			(this->*ChunkNames[n].FuncV2)(f, posInFileNow, Len);
		}
		
	}

	posInFileNow = posInFile + sizeof(CHUNK_HEADER_LONG) + Len;

	// account for odd alignment
	if(posInFileNow & 0x1)
		posInFileNow++;

	fseek(f, posInFileNow, SEEK_SET);

	return posInFileNow;
}

/**
reads the subchunk.
from this file position, executes this patch
@param f file handle
@param posInFile current position
@param maxSize the end of this chunk
@return position afterwards
*/
long LWO_Loader::dispatchSubChunk(FILE *f, long posInFile,long maxSize)
{
	long posInFileNow;
	int n;

	UNREFERENCED_PARAMETER( maxSize );

	// read chunk name
	ID4 ChunkName = read(f, ChunkName );
	// read length of this data segment
	U2 Len = read(f, Len);

	n = findSubChunk( *((CHUNK_ENUM*) &ChunkName) );

	if(n < 0)
	{
		LOG(INFO) << "Unknown Chunk : "	<< ((unsigned char*)&ChunkName)[0] 
									<< ((unsigned char*)&ChunkName)[1] 
									<< ((unsigned char*)&ChunkName)[2] 
									<< ((unsigned char*)&ChunkName)[3] 
									<< "\n";
	} else
	{
		// replaced later but used for subchunks etc
		posInFileNow = posInFile + sizeof(CHUNK_HEADER_SHORT);

		if(lwoVersion == 1)
		{
//			if( ChunkNames[n].FuncV1 == LWO_Loader::SkipReader)
//				LOG(INFO) << "Skipping Chunk : " << *((unsigned int*)&ChunkName) << "\n";

			(this->*SubChunkNames[n].FuncV1)(f, posInFileNow, Len);
		}
		else
		{
//			if( ChunkNames[n].FuncV2 == LWO_Loader::SkipReader)
//				LOG(INFO) << "Skipping Chunk : " << *((unsigned int*)&ChunkName) << "\n";

			(this->*SubChunkNames[n].FuncV2)(f, posInFileNow, Len);
		}
		
	}

	posInFileNow = posInFile + sizeof(CHUNK_HEADER_SHORT) + Len;

	// account for odd alignment
	if(posInFileNow & 0x1)
		posInFileNow++;

	fseek(f, posInFileNow, SEEK_SET);

	return posInFileNow;
}

/**
read all chunks.
from this file position, dispatchs all chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::SubChunkReader(FILE *f, long currentPos, long maxSize)
{
	long endPos = ftell(f) + maxSize;

	while( ftell(f) < endPos )
	{
		currentPos = dispatchSubChunk(f,currentPos,maxSize);
	}
}


/**
read all chunks.
from this file position, dispatchs all chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::ChunkReader(FILE *f, long currentPos, long maxSize)
{
	while( ftell(f) < maxSize )
	{
		currentPos = dispatchChunk(f,currentPos,maxSize);
	}
}

/**
Skip Chunk.
A Null chunk, just ignores it by skipping over it
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::SkipReader(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

/**
FORM Chunk.
Checks this a LWO file and wether V1 or V2
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::FORMReader(FILE *f, long p, long maxSize)
{
	int Id;

	if( fread(&Id, sizeof(Id), 1, f) != 1 )
		CoreThrowException(FileError,"");

	if(Id == CHUNK_NAME('L','W','O','B'))
	{
		//LWO
		lwoVersion = 1;
		assert( false );
		ChunkReader(f, p + 4, maxSize );
	}
	else if (Id == CHUNK_NAME('L','W','O','2'))
	{
		// LWO2
		lwoVersion = 2;
		ChunkReader(f, p + 4 , maxSize );
	}
	else
	{
		// unknown FORM type
		CoreThrowException(LightWaveLoaderFileError,"");
	}

}

/**
TAGS Chunk V2.
Stores the V2 Tags
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::TAGSReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	// read the tags list
	long startPos = ftell(f);
	long curPos = ftell(f);
	
	// loop round reading tags
	while( (curPos - startPos) < maxSize )
	{
		S0 temp;
		read(f,temp);
		tags.push_back( temp );

		curPos = ftell(f);
	}
}

/**
LAYR Chunk V2.
Decodes the LWO2 layer chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::LAYRReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startChunk = ftell(f);
	U2 number = read(f,number);
	U2 flags = read(f,flags);
	VEC12 pivot = read(f,pivot);
	S0 name;
	read(f,name);
	long endChunk = ftell(f);
	
	U2 parent = 0xFFFF;
	// check for optional parent part
	if( (endChunk - startChunk) != maxSize )
	{
		parent = read(f,parent);
	}
	
	// allocate a new layer and set it current
	layers.push_back( Layer() );
	curLayer = &layers.back();

	// copy data across
	curLayer->number = number;
	if( flags & 0x1 )
		curLayer->flags = Layer::HIDDEN;
	curLayer->pivot = pivot;
	curLayer->name = name;
	if(parent != 0xFFFF )
		curLayer->parent = parent;

}

/**
PNTS Chunk V2.
Decodes the LWO2 point chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::PNTSReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	// check we have a current layer if not create one
	if( curLayer == 0)
	{
		// allocate a new layer and set it current
		layers.push_back( Layer() );
		curLayer = &layers.back();
	}

	unsigned int numPoints = maxSize/sizeof(VEC12);
	curLayer->points.reserve( numPoints + curLayer->points.size() );

	VEC12 temp;
	for(unsigned int count = 0; count < numPoints;count++)
	{
		// read a vector
		read( f, temp );
		/// adds a new vertex position to the array
		curLayer->points.push_back( temp );
	}
}

/**
POLS Chunk V2.
Decodes the LWO2 polygon chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::POLSReader2(FILE *f, long p, long maxSize)
{
	char type[4];
	
	read(f, *((ID4*)type) );

	switch( *((int*)type) )
	{
		case CHUNK_NAME('P','T','C','H'):
		case CHUNK_NAME('F','A','C','E'):
			POLS2Face(f,p,maxSize - 4);
			break;
		default:
			LOG(INFO) << "Unknown PolyType : " << type[0] << type[1] << type[2] << type[3] << "\n";
			break;
	}
}

/**
POLS V2 Face type.
Decodes the LWO2 polygon chunk of sub type FACE
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::POLS2Face(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);
	long curPos = ftell(f);
	
	// loop round reading polygons
	while( (curPos - startPos) < maxSize )
	{
		// 2 byte index, top 6 bit flags
		U2 numVerts = read(f, numVerts);
		U2 flags = numVerts;
		flags &= 0xF600; // top 6 bit
		numVerts &= ~0xF600; // bottom 12 bits
		
		assert(numVerts < 1023);
		assert(flags == 0); // no flags defined for face polygons

		std::vector<VX> indices;
		for(int i = 0; i < numVerts;i++)
		{
			VX index = readVX(f, index);
			indices.push_back( index );
		}
		
		curLayer->faces.push_back( Face(indices) );

		curPos = ftell(f);
	}

}

void LWO_Loader::PTAGReader2(FILE *f, long p, long maxSize)
{
	char type[4];
	
	read(f, *((ID4*)type) );

	switch( *((int*)type) )
	{
		case CHUNK_SURF:
			PTAG2Surf(f,p,maxSize - 4);
			break;
		case CHUNK_PART:
			PTAG2Part(f,p,maxSize - 4);
			break;
		case CHUNK_SMGP:
			PTAG2Smgp(f,p,maxSize - 4);
			break;
		default:
			LOG(INFO) << "Unknown PolyTag Type : " << type[0] << type[1] << type[2] << type[3] << "\n";
			break;
	}
}

/**
PTAG V2 type SURF.
Decodes the LWO2 PTag chunk of sub type SURF
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::PTAG2Surf(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	assert(curLayer != 0);

	long startPos = ftell(f);
	long curPos = ftell(f);
	
	// loop round reading polygons
	while( (curPos - startPos) < maxSize )
	{
		VX poly = readVX(f, poly);

		// 2 byte index
		U2 tag = read(f, tag);

		PolyTag stag;
		stag.polygon = poly;
		stag.tagID = tag;
		curLayer->surfaceTag2Poly.push_back( stag );

		curPos = ftell(f);
	}

}

/**
PTAG V2 type PART.
Decodes the LWO2 PTag chunk of sub type PART
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::PTAG2Part(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	assert(curLayer != 0);

	long startPos = ftell(f);
	long curPos = ftell(f);
	
	// loop round reading polygons
	while( (curPos - startPos) < maxSize )
	{
		VX poly = readVX(f, poly);

		// 2 byte index
		U2 tag = read(f, tag);

		PolyTag stag;
		stag.polygon = poly;
		stag.tagID = tag;
		curLayer->partTag2Poly.push_back( stag );

		curPos = ftell(f);
	}

}

/**
PTAG V2 type SMGP.
Decodes the LWO2 PTag chunk of sub type SMGP
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::PTAG2Smgp(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	assert(curLayer != 0);

	long startPos = ftell(f);
	long curPos = ftell(f);
	
	// loop round reading polygons
	while( (curPos - startPos) < maxSize )
	{
		VX poly = readVX(f, poly);

		// 2 byte index
		U2 tag = read(f, tag);

		PolyTag stag;
		stag.polygon = poly;
		stag.tagID = tag;
		curLayer->smgpTag2Poly.push_back( stag );

		curPos = ftell(f);
	}

}

/**
VMAP Chunk V2.
Decodes the LWO2 vertex map chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAPReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	char type[4];
	
	read(f, *((ID4*)type) );

	switch( *((int*)type) )
	{
		case CHUNK_NAME('P','I','C','K'):
			VMAP2Pick(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('W','G','H','T'):
			VMAP2Wght(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('M','N','V','W'):
			VMAP2Mnvw(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('T','X','U','V'):
			VMAP2Txuv(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('R','G','B',' '):
			VMAP2Rgb(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('R','G','B','A'):
			VMAP2Rgba(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('M','O','R','F'):
			VMAP2Morf(f,p,maxSize -4);
			break;
		case CHUNK_NAME('S','P','O','T'):
			VMAP2Spot(f,p,maxSize - 4);
			break;
		default:
			LOG(INFO) << "Unknown VMAPTag Type : " << type[0] << type[1] << type[2] << type[3] << "\n";
			break;
	}
}
/**
VMAP Pick type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Pick(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for pick maps dimension should be 0
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 0);

	assert( curLayer != 0 );

	PickMap* map = new PickMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index
		VX index = readVX( f, index );
		map->picks.push_back( index );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP Weight Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Wght(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for weight maps dimension should be 1
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 1);

	assert( curLayer != 0 );

	WeightMap* map = new WeightMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index and a single float
		VX index = readVX( f, index );
		float weight = read(f, weight );

		map->weights.push_back( WeightMap::Weight(index,weight) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP Subpatch weight Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Mnvw(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

/**
VMAP Texture uv Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Txuv(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	assert( curLayer != 0 );

	// for uv maps dimension should be 2
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 2);

	UVMap* map = new UVMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index and a single float
		VX index = readVX( f, index );
		float u = read(f, u );
		float v = read(f, v );

		map->uvs.push_back( UVMap::UV(index,u,v) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP RGB Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Rgb(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for uv maps dimension should be 3
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 3);

	assert( curLayer != 0 );

	RGBMap* map = new RGBMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index and 3 floats
		VX index = readVX( f, index );
		float r = read(f, r );
		float g = read(f, g );
		float b = read(f, b );

		map->rgbs.push_back( RGBMap::RGBFloat(index,r,g,b) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP RGBA Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Rgba(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for uv maps dimension should be 4
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 4);

	assert( curLayer != 0 );

	RGBAMap* map = new RGBAMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index and 4 floats
		VX index = readVX( f, index );
		float r = read(f, r );
		float g = read(f, g );
		float b = read(f, b );
		float a = read(f, a );

		map->rgbas.push_back( RGBAMap::RGBAFloat(index,r,g,b,a) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP Morf Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Morf(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for uv maps dimension should be 3
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 3);

	assert( curLayer != 0 );

	MorfMap* map = new MorfMap;
	curLayer->vertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX index and 3 floats
		VX index = readVX( f, index );
		float x = read(f, x );
		float y = read(f, y );
		float z = read(f, z );

		map->deltas.push_back( MorfMap::VertexDelta(index,x,y,z) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP Spot Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAP2Spot(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

/**
VMAD Chunk V2.
Decodes the LWO2 vertex map chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMADReader2(FILE *f, long p, long maxSize)

{
	UNREFERENCED_PARAMETER( p );

	char type[4];
	
	read(f, *((ID4*)type) );

	switch( *((int*)type) )
	{
		case CHUNK_NAME('P','I','C','K'):
			VMAD2Pick(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('W','G','H','T'):
			VMAD2Wght(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('M','N','V','W'):
			VMAD2Mnvw(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('T','X','U','V'):
			VMAD2Txuv(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('R','G','B',' '):
			VMAD2Rgb(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('R','G','B','A'):
			VMAD2Rgba(f,p,maxSize - 4);
			break;
		case CHUNK_NAME('S','P','O','T'):
			VMAD2Spot(f,p,maxSize - 4);
			break;
		default:
			LOG(INFO) << "Unknown VMADTag Type : " << type[0] << type[1] << type[2] << type[3] << "\n";
			break;
	}
}


void LWO_Loader::VMAD2Pick(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}
/**
VMAP Weight Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Wght(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for weight maps dimension should be 1
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 1);

	assert( curLayer != 0 );

	WeightDMap* map = new WeightDMap;
	curLayer->discVertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX vertex index and VX poly index and a single float
		VX vIndex = readVX( f, vIndex );
		VX pIndex = readVX( f, pIndex );
		float weight = read(f, weight );

		map->weights.push_back( WeightDMap::Weight(vIndex,pIndex,weight) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}
/**
VMAP Subpatch weight Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Mnvw(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	LOG(INFO) << "VMAD of type Mnvw not supported";

}
/**
VMAP Texture uv Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Txuv(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for weight maps dimension should be 2
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 2);

	assert( curLayer != 0 );

	UVDMap* map = new UVDMap;
	curLayer->discVertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX vertex index and VX poly index and 2 floats
		VX vIndex = readVX( f, vIndex );
		VX pIndex = readVX( f, pIndex );
		assert( vIndex < curLayer->points.size() );
		assert( pIndex < curLayer->faces.size() );
		float u = read(f, u );
		float v = read(f, v );

		map->uvs.push_back( UVDMap::UV(vIndex,pIndex,u,v) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP RGB Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Rgb(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for weight maps dimension should be 3
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 3);

	assert( curLayer != 0 );

	RGBDMap* map = new RGBDMap;
	curLayer->discVertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX vertex index and VX poly index and 3 floats
		VX vIndex = readVX( f, vIndex );
		VX pIndex = readVX( f, pIndex );
		float r = read(f, r );
		float g = read(f, g );
		float b = read(f, b );

		map->rgbs.push_back( RGBDMap::RGBFloat(vIndex,pIndex,r,g,b) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP RGBA Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Rgba(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	// for weight maps dimension should be 4
	U2 dim = read( f, dim );

	UNREFERENCED_PARAMETER( dim );
	assert(dim == 4);

	assert( curLayer != 0 );

	RGBADMap* map = new RGBADMap;
	curLayer->discVertexMaps.push_back( map );

	// vertex map name
	read(f, map->name);

	long curPos = ftell(f);

	for(long count = 0; count < maxSize - (curPos - startPos); )
	{
		long mark0 = ftell(f);

		// read in VX vertex index and VX poly index and 4 floats
		VX vIndex = readVX( f, vIndex );
		VX pIndex = readVX( f, pIndex );
		float r = read(f, r );
		float g = read(f, g );
		float b = read(f, b );
		float a = read(f, a );

		map->rgbas.push_back( RGBADMap::RGBAFloat(vIndex,pIndex,r,g,b,a) );

		long mark1 = ftell(f);
		count += (mark1 - mark0);
	}
}

/**
VMAP Morf Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Morf(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	LOG(INFO) << "VMAD of type Morf not supported";
}

/**
VMAP Spot Map type.
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::VMAD2Spot(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	LOG(INFO) << "VMAD of type Spot not supported";
}

/**
VMAD Chunk V2.
Decodes the LWO2 vertex map chunk
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/

//---------------------------------------------------------------------
// CLIP segment
//---------------------------------------------------------------------

/**
CLIP Chunk V2.
Decodes a clip chunk (a bitmap or avi)
@param f file handle
@param currentPos current position
@param maxSize the end of this chunk
*/
void LWO_Loader::CLIPReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );

	long startPos = ftell(f);

	U4 index = read( f, index );

	long curPos = ftell(f);

	clip.push_back( Clip() );
	curClip = &clip.back();

	curClip->id = index;

	SubChunkReader(f, curPos, maxSize - (curPos - startPos) );

	curClip = 0;
}

void LWO_Loader::STILReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert( curClip != 0 );

	S0 filename;
	read( f, filename );

	curClip->type = LightWave::Clip::STIL;
	curClip->filename = filename;
}

//---------------------------------------------------------------------
// SURF segment
//---------------------------------------------------------------------

//---------------------------------------------------------------------
void LWO_Loader::SURFReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	long startPos = ftell(f);
	// get surface name
	S0 name;
	read(f, name);
//	LOG(INFO) << "Surface Found : " << name << "\n";

	S0 parentSurf;
	read(f, parentSurf);
	if( !parentSurf.empty() )
		LOG(INFO) << "	Parent : " << parentSurf << "\n";

	// assert we only have 'closed' the last surface we worked on
	assert(curSurface == 0);
	curSurface = new Surface;
	curSurface->name = name;

	// a surface name with out tagId NOT allowed
	assert( findTagId(name) != -1);

	curSurface->tagID = findTagId(name);
	surfaceMap[ curSurface->tagID ] = curSurface;

	long curPos = ftell(f);

	SubChunkReader(f, curPos, maxSize - (curPos - startPos) );

	// memory will be freed when object is destroyed
	curSurface = 0;
}

//---------------------------------------------------------------------
void LWO_Loader::COLRReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	COL12 colr = read(f, colr);
	curSurface->baseColour = colr;
}

//---------------------------------------------------------------------
void LWO_Loader::DIFFReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->diffuse = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::LUMIReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->luminosity = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::SPECReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->specular = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::REFLReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->reflection = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::TRANReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->transparency = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::TRNLReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);
	curSurface->translunency = diff;
}

//---------------------------------------------------------------------
void LWO_Loader::GLOSReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 diff = read(f, diff);

	curSurface->glossiness = diff;
}
//---------------------------------------------------------------------
void LWO_Loader::SIDEReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	U2 side = read(f, side);

	curSurface->doubleSided = (side == 3) ? true : false;
}
//---------------------------------------------------------------------
void LWO_Loader::SMANReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface != 0);

	FP4 sman = read(f, sman);

	curSurface->maxSmoothingAngle = sman;
}


//---------------------------------------------------------------------
// BLOK segment
//---------------------------------------------------------------------

//---------------------------------------------------------------------
void LWO_Loader::BLOKReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	long startPos = ftell(f);

	// new block
	curSurface->blocks.push_back( LWBlok() );
	curSurface->curBlock = &curSurface->blocks.back();

	long curPos = ftell(f);
	SubChunkReader(f, curPos, maxSize - (curPos - startPos) );

	curSurface->curBlock = 0;
}

//---------------------------------------------------------------------
void LWO_Loader::IMAPReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	long startPos = ftell(f);

	// read the ordinal 
	S0 ordinal;
	read(f,ordinal);
//	LOG(INFO) << "Image Map \n";

	curSurface->curBlock->ordinal = ordinal;
	curSurface->curBlock->type = LWBlok::IMAP;

	long curPos = ftell(f);
	SubChunkReader(f, curPos, maxSize - (curPos - startPos) );
}

//---------------------------------------------------------------------
void LWO_Loader::CHANReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	char type[4];
	
	read(f, *((ID4*)type) );

	switch( *((int*)type) )
	{
		case CHUNK_COLR:
			curSurface->curBlock->channel = LWBlok::COLR;
			break;
		case CHUNK_DIFF:
			curSurface->curBlock->channel = LWBlok::DIFF;
			break;
		case CHUNK_LUMI:
			curSurface->curBlock->channel = LWBlok::LUMI;
			break;
		case CHUNK_SPEC:
			curSurface->curBlock->channel = LWBlok::SPEC;
			break;
		case CHUNK_GLOS:
			curSurface->curBlock->channel = LWBlok::GLOS;
			break;
		case CHUNK_REFL:
			curSurface->curBlock->channel = LWBlok::REFL;
			break;
		case CHUNK_TRAN:
			curSurface->curBlock->channel = LWBlok::TRAN;
			break;
		case CHUNK_RIND:
			curSurface->curBlock->channel = LWBlok::RIND;
			break;
		case CHUNK_TRNL:
			curSurface->curBlock->channel = LWBlok::TRNL;
			break;
		case CHUNK_BUMP:
			curSurface->curBlock->channel = LWBlok::BUMP;
			break;
		default:
			LOG(INFO) << "Unknown Channel Type : " << type[0] << type[1] << type[2] << type[3] << "\n";
			break;
	}
}

//---------------------------------------------------------------------
void LWO_Loader::ENABReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	U2 enable = read(f,enable);

	curSurface->curBlock->enable = (enable == 0) ? false : true;

}

//---------------------------------------------------------------------
void LWO_Loader::AXISReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	U2 axis = read(f,axis);

	assert( axis <= 3); // allowable valuses

	switch( axis )
	{
	case 0:
		curSurface->curBlock->axis = LWBlok::X_AXIS;
		break;
	case 1:
		curSurface->curBlock->axis = LWBlok::Y_AXIS;
		break;
	case 2:
		curSurface->curBlock->axis = LWBlok::Z_AXIS;
		break;
	default:
		break;
	}
}

//---------------------------------------------------------------------
void LWO_Loader::TMAPReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	long startPos = ftell(f);

	long curPos = ftell(f);
	SubChunkReader(f, curPos, maxSize - (curPos - startPos) );

}

//---------------------------------------------------------------------
void LWO_Loader::PROJReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	U2 project = read(f,project);

	assert( project <= 5); // allowable valuses

	switch( project )
	{
	case 0:
		curSurface->curBlock->projectionMode = LWBlok::PLANAR;
		break;
	case 1:
		curSurface->curBlock->projectionMode = LWBlok::CYLINDRICAL;
		break;
	case 2:
		curSurface->curBlock->projectionMode = LWBlok::SPHERICAL;
		break;
	case 3:
		curSurface->curBlock->projectionMode = LWBlok::CUBIC;
		break;
	case 4:
		curSurface->curBlock->projectionMode = LWBlok::FRONT_PROJECTION;
		break;
	case 5:
		curSurface->curBlock->projectionMode = LWBlok::UV;
		break;
	default:
		break;
	}


}

//---------------------------------------------------------------------
void LWO_Loader::CNTRReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	VEC12 center = read(f, center);
	
	curSurface->curBlock->center = center;
}

//---------------------------------------------------------------------
void LWO_Loader::SIZEReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	VEC12 size = read(f, size);
	
	curSurface->curBlock->size = size;

}

//---------------------------------------------------------------------
void LWO_Loader::ROTAReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	VEC12 rota = read(f, rota);
	
	curSurface->curBlock->rotation = rota;

}

//---------------------------------------------------------------------
void LWO_Loader::CSYSReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	U2 type = read(f, type);
	
	bool enable = (type == 0) ? true : false;
	curSurface->curBlock->objectSpace = enable;

}

//---------------------------------------------------------------------
void LWO_Loader::IMAGReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	VX index = readVX(f, index);
	
	curSurface->curBlock->imageIndex = index;

}

//---------------------------------------------------------------------
void LWO_Loader::VMAPSubReader2(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );

	assert(curSurface->curBlock != 0);

	S0 vmapName;
	read(f, vmapName );
	
	curSurface->curBlock->vmapName = vmapName;

}

//----------------------------------------------------------------------
void LWO_Loader::PNTSReader(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

void LWO_Loader::SRFSReader(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

void LWO_Loader::POLSReader(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

void LWO_Loader::SURFReader(FILE *f, long p, long maxSize)
{
	UNREFERENCED_PARAMETER( f );
	UNREFERENCED_PARAMETER( p );
	UNREFERENCED_PARAMETER( maxSize );
}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
