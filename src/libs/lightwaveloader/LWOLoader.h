/** \file LWOLoader.h
   LWO loader header file.
   Loads LWO (Lightwave 7)
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_LOADER_LWOLOAD_H_ )
#define LIGHTWAVE_LOADER_LWOLOAD_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <cstdio>
#include <string>
#include <map>
#include <vector>
#include "LWTypes.h"
#include "LWSurface.h"
#include "LWLayer.h"
#include "LWClip.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------
	DECLARE_EXCEPTION( LightWaveLoaderFileError, A error occured loading a LightWave object );

	/**
	Short description.
	Detailed description
	*/
	class LWO_Loader
	{
	private:

	public:	
		LWO_Loader( const std::string& filename );
		~LWO_Loader();

		/// LW object 1 or 2
		int lwoVersion;

		std::vector<std::string>		tags;				///< string tag table
		std::vector<Layer*>				layers;			///< layer table
		std::vector<Clip>				clip;				///< Clip table
		std::map<unsigned int, Surface*> surfaceMap;
		VEC12							bboxMin;
		VEC12							bboxMax;
	private:
		Layer*				curLayer;		///< current layer
		Clip*				curClip;		///< current clip
		Surface*			curSurface;	///< current surface
		
		void SortSurfaceBlocks();

	public: // most of this is actually private but due to the way the table look up work
			// it has to public
		int findTagId(std::string& tag);

		long dispatchChunk(FILE *f, long posInFile,long maxSize);
		long dispatchSubChunk(FILE *f, long posInFile,long maxSize);

		void ChunkReader(FILE *f, long cur, long max);
		void SubChunkReader(FILE *f, long cur, long max);

		void SkipReader(FILE *f, long p, long maxSize);

		// top level chunks
		void FORMReader(FILE *f, long p, long maxSize);
		void PNTSReader(FILE *f, long p, long maxSize);
		void SRFSReader(FILE *f, long p, long maxSize);
		void POLSReader(FILE *f, long p, long maxSize);
		void SURFReader(FILE *f, long p, long maxSize);

		// top level chunks V2
		void TAGSReader2(FILE *f, long p, long maxSize);
		void LAYRReader2(FILE *f, long p, long maxSize);
		void PNTSReader2(FILE *f, long p, long maxSize);
		void BBOXReader2(FILE *f, long p, long maxSize);

		// vertex maps
		void VMAPReader2(FILE *f, long p, long maxSize);
		void VMAP2Pick(FILE *f, long p, long maxSize);
		void VMAP2Wght(FILE *f, long p, long maxSize);
		void VMAP2Mnvw(FILE *f, long p, long maxSize);
		void VMAP2Txuv(FILE *f, long p, long maxSize);
		void VMAP2Rgb(FILE *f, long p, long maxSize);
		void VMAP2Rgba(FILE *f, long p, long maxSize);
		void VMAP2Morf(FILE *f, long p, long maxSize);
		void VMAP2Spot(FILE *f, long p, long maxSize);
		
		// discontinous vertex maps
		void VMADReader2(FILE *f, long p, long maxSize);
		void VMAD2Pick(FILE *f, long p, long maxSize);
		void VMAD2Wght(FILE *f, long p, long maxSize);
		void VMAD2Mnvw(FILE *f, long p, long maxSize);
		void VMAD2Txuv(FILE *f, long p, long maxSize);
		void VMAD2Rgb(FILE *f, long p, long maxSize);
		void VMAD2Rgba(FILE *f, long p, long maxSize);
		void VMAD2Morf(FILE *f, long p, long maxSize);
		void VMAD2Spot(FILE *f, long p, long maxSize);
		
		// polygons
		void POLSReader2(FILE *f, long p, long maxSize);
		void POLS2Face(FILE *f, long p, long maxSize);

		// poly tags
		void PTAGReader2(FILE *f, long p, long maxSize);
		void PTAG2Surf(FILE *f, long p, long maxSize);
		void PTAG2Part(FILE *f, long p, long maxSize);
		void PTAG2Smgp(FILE *f, long p, long maxSize);

		void SURFReader2(FILE *f, long p, long maxSize);
		
		void COLRReader2(FILE *f, long p, long maxSize);
		void DIFFReader2(FILE *f, long p, long maxSize);
		void LUMIReader2(FILE *f, long p, long maxSize);
		void SPECReader2(FILE *f, long p, long maxSize);
		void REFLReader2(FILE *f, long p, long maxSize);
		void TRANReader2(FILE *f, long p, long maxSize);
		void TRNLReader2(FILE *f, long p, long maxSize);
		void GLOSReader2(FILE *f, long p, long maxSize);
		void BLOKReader2(FILE *f, long p, long maxSize);
		void IMAPReader2(FILE *f, long p, long maxSize);
		void CHANReader2(FILE *f, long p, long maxSize);
		void ENABReader2(FILE *f, long p, long maxSize);
		void AXISReader2(FILE *f, long p, long maxSize);
		void TMAPReader2(FILE *f, long p, long maxSize);
		void PROJReader2(FILE *f, long p, long maxSize);
		void CNTRReader2(FILE *f, long p, long maxSize);
		void SIZEReader2(FILE *f, long p, long maxSize);
		void ROTAReader2(FILE *f, long p, long maxSize);
		void CSYSReader2(FILE *f, long p, long maxSize);
		void IMAGReader2(FILE *f, long p, long maxSize);
		void SIDEReader2(FILE *f, long p, long maxSize);
		void SMANReader2(FILE *f, long p, long maxSize);
	
		void CLIPReader2(FILE *f, long p, long maxSize);
		void STILReader2(FILE *f, long p, long maxSize);

		void VMAPSubReader2(FILE *f, long p, long maxSize);
	}; 
//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

} // end namespace
//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
