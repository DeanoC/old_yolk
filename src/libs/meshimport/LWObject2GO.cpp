/** \file GOMesh.cpp
   Geometry Optimise Mesh implementation.
   (c) 2002 Deano Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "meshimport.h"

#include "meshmod/mesh.h"
#include "lightwaveloader/LWObject.h"
#include "core/mathconsts.h"
#include "core/line.h"
#include "core/file_path.h"
#include "meshmod/uvvertex.h"
#include <meshmod/materialface.h>
#include <algorithm>

#include "LWObject2GO.h"

#define UNREFERENCED_PARAMETER(x) (void)x

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------
namespace LWObject2Go_Local
{
	void AddLWOLayers(	const LightWave::Object& in, 
						MeshMod::MeshPtr out, 
						const unsigned int layer, 
						const Math::Matrix4x4& transform );
	void AddLWOMaterials( const LightWave::Object& in, MeshMod::MeshPtr out );

	std::map<unsigned int, unsigned int> LWSurfTagToMatNum;
	void ConvertSurfaceTagToMatNum( MeshMod::MeshPtr out );

	std::map<unsigned int, std::string> LWIdToFileName;
	void AddLWOClips( const LightWave::Object& in, MeshMod::MeshPtr out );

	void AddLWLayerVertices( const LightWave::Layer& in, MeshMod::MeshPtr out, const Math::Matrix4x4& transform, const Math::Vector3& pivot );
	void AddLWLayerPolys( const LightWave::Layer& in, MeshMod::MeshPtr out, 
							const unsigned int baseVertex );
	void AddLWLayerPolyTags( const LightWave::Layer& in, MeshMod::MeshPtr out, const unsigned int baseFace );
	void AddLWLayerVMap( const LightWave::Layer& in, MeshMod::MeshPtr out, 
							const unsigned int baseVertex );
	void AddLWLayerDVMap( const LightWave::Layer& in, MeshMod::MeshPtr out, 
							const unsigned int baseVertex, const unsigned baseFace );
	void AddLWOSurface( const LightWave::Surface* curSurface, MeshMod::MeshPtr out );

	struct LWSurfPolyTag
	{
		unsigned short tagId;

		LWSurfPolyTag() : tagId(0) {};
		LWSurfPolyTag(unsigned short id) : 
			tagId(id)
		{}

		static const std::string getName() { return "LWSurfPolyTag"; };
	};

	typedef MeshMod::ImplElements< LWSurfPolyTag, MeshMod::Face_ > LWSurfPolyTagElements;

	struct LWSmgpPolyTag
	{
		unsigned short tagId;

		LWSmgpPolyTag() : tagId(0) {};
		LWSmgpPolyTag(unsigned short id) : 
			tagId(id)
		{}

		static const std::string getName() { return "LWSmgpPolyTag"; };
	};
	typedef MeshMod::ImplElements< LWSmgpPolyTag, MeshMod::Face_ > LWSmgpPolyTagElements;

	float xyztoh(float x,float y,float z);
	Math::Vector2 generatePlanarUV( const Math::Vector3& in, const LightWave::LWBlok& block );
	Math::Vector2 generateCylinderUV( const Math::Vector3& in, const LightWave::LWBlok& block );

	//----------------- Bones -----------------------
	float generateBoneWeightForVertex(		const LightWave::Bone* bone, 
											const float inWeight, 
											const Math::Vector3& pt,
											const float fallExponent  );
	void AddLWOBones(const LightWave::Object& object, MeshMod::MeshPtr out, const unsigned int baseVertex );

	bool BoneWeightSort( const MeshMod::VertexData::BoneWeights::Weight& a, const MeshMod::VertexData::BoneWeights::Weight& b )
	{
		return (a.weight > b.weight);
	}

}

using namespace LWObject2Go_Local;

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------
using namespace MeshMod;

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

namespace MeshImport
{


/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
MeshMod::MeshPtr LightWaveObject2GoMesh(	const LightWave::Object& in, 
									MeshMod::MeshPtr out, 
									const unsigned int layer,
									const Math::Matrix4x4& transform )
{
	// clear material tag lookup table
	LWSurfTagToMatNum.clear();
	LWIdToFileName.clear();

	if( !out ) {
		out = std::make_shared<MeshMod::Mesh>( Core::FilePath( in.name ).BaseName().value() );
	}

	AddLWOLayers( in, out, layer, transform );

	AddLWOClips( in, out );

	AddLWOMaterials( in, out );

	ConvertSurfaceTagToMatNum( out );

	// join pair edges
	out->connectPairEdges();

	return out;
}

} // end namespace GO

namespace LWObject2Go_Local
{
	float xyztoh(float x,float y,float z) {
		UNREFERENCED_PARAMETER(y);

		float h;

		if (x == 0.0 && z == 0.0)
			h = 0.0;
		else {
			if (z == 0.0)
				h = ((x < 0.0) ? Math::pi_over_2<float>() : -Math::pi_over_2<float>() );
			else if (z < 0.0)
				h = -(float)atan(x / z) + Math::pi<float>();
			else
				h = -(float)atan(x / z);
		}
		
		return h;
	}

	Math::Vector2 generatePlanarUV( const Math::Vector3& in, const LightWave::LWBlok* block ) {

		float x,y,z, s,t;

		x = in[0] - block->center[0];
		y = in[1] - block->center[1];
		z = in[2] - block->center[2];

		switch( block->axis )
		{
		case LightWave::LWBlok::X_AXIS:
			s = z / block->size[2] + 0.5f;
			t = -y / block->size[1] + 0.5f;
			break;
		case LightWave::LWBlok::Y_AXIS:
			s = x / block->size[0] + 0.5f;
			t = -z / block->size[2] + 0.5f;
			break;
		case LightWave::LWBlok::Z_AXIS:
			s = x / block->size[0] + 0.5f;
			t = -y / block->size[1] + 0.5f;
			break;
		default:
			assert(false);
			s = t = 0.f;
			break;
		}

		return Math::Vector2(s,t);
	}

	Math::Vector2 generateCylinderUV( const Math::Vector3& in, const LightWave::LWBlok* block ) {

		float x,y,z,u,v,lon,t;
		x = in[0] - block->center[0];
		y = in[1] - block->center[1];
		z = in[2] - block->center[2];

		switch( block->axis )
		{
		case LightWave::LWBlok::X_AXIS:
			lon = xyztoh(z,x,-y);
			t = -x / block->size[0] + 0.5f;
			break;
		case LightWave::LWBlok::Y_AXIS:
			lon = xyztoh(-x,y,z);
			t = -y / block->size[1] + 0.5f;
			break;
		case LightWave::LWBlok::Z_AXIS:
			lon = xyztoh(-x,z,-y);
			t = -z / block->size[2] + 0.5f;
			break;
		default:
			assert(false);
			lon = t = false;
		}

		lon = 1.0f - lon / (Math::pi<float>() * 2.0f);
		u = lon;
		v = t;

		return Math::Vector2(u,v);
	}

	std::string UVChannelNameFromLWBlok( const LightWave::LWBlok* block )
	{
		std::string name = "TXUV_";
		switch( block->projectionMode ) {
			case LightWave::LWBlok::PLANAR:
				name += "Planar_";
				break;
			case LightWave::LWBlok::CYLINDRICAL:
				name += "Cylindrical_";
				break;
			case LightWave::LWBlok::SPHERICAL:
				name += "Spherical_";
				break;
			case LightWave::LWBlok::UV:
				// just "TXUV_" + m_vmapName
				name += block->vmapName;
				return name;
				break;
		}
		switch( block->axis ) {
			case LightWave::LWBlok::X_AXIS:
				name += "XAxis_";
				break;
			case LightWave::LWBlok::Y_AXIS:
				name += "YAxis_";
				break;
			case LightWave::LWBlok::Z_AXIS:
				name += "ZAxis_";
				break;
		}
		char sizeText[1024];
		char centerText[1024];
		sprintf( sizeText, "%f_%f_%f_", block->size[0], block->size[1], block->size[2] );
		sprintf( centerText, "%f_%f_%f_", block->center[0], block->center[1], block->center[2] );

		name += sizeText;
		name += centerText;

		return name;
	}

	std::string generateUV( const LightWave::LWBlok* block, MeshMod::MeshPtr out ) {
		// were we going to add the vmap elements
		MeshMod::VertexElementsContainer& vertCon = out->getVertexContainer();

		// form name from parameters
		std::string uvName = UVChannelNameFromLWBlok( block );

		// generate for all UV if channel doesn't already exist
		MeshMod::UVVertexElements* uvEle = vertCon.getElements<MeshMod::UVVertexElements>( uvName );
		if( uvEle == 0) {
			bool errLogged = false;
			uvEle = vertCon.addElements<MeshMod::UVVertexElements>( uvName );
			MeshMod::PositionVertexElements* posEle = vertCon.getElements<MeshMod::PositionVertexElements>();

			std::vector< VertexData::UV >::iterator uvIt = uvEle->elements.begin();
			while( uvIt != uvEle->elements.end() ) {
				const VertexIndex vertNum = (VertexIndex) (uvIt - uvEle->elements.begin() );

				Math::Vector3 pos = Math::Vector3( (*posEle)[vertNum].x, (*posEle)[vertNum].y, (*posEle)[vertNum].z );
				Math::Vector2 uv;
				switch( block->projectionMode ) {
					case LightWave::LWBlok::PLANAR:
						uv = generatePlanarUV( pos, block );
						(*uvIt).u = uv[0];
						(*uvIt).v = uv[1];
						break;
					case LightWave::LWBlok::CYLINDRICAL:
						uv = generateCylinderUV( pos, block );
						(*uvIt).u = uv[0];
						(*uvIt).v = uv[1];
						break;
					case LightWave::LWBlok::SPHERICAL:
					default:
						if( errLogged == false ) {
							LOG(INFO) << block->vmapName << " : " << uvName << 
													" uses spherical UV generation which is not supported\n";
							errLogged = true;
						}
						(*uvIt).u = 0.0f;
						(*uvIt).v = 0.0f;
						break;
					case LightWave::LWBlok::UV:
						if( errLogged == false ) {
							LOG(INFO) << block->vmapName << " : " << uvName << 
													" shouldn't get here (it means we haven't get a uv map?)\n";
							errLogged = true;
						}
						(*uvIt).u = 0.0f;
						(*uvIt).v = 0.0f;
						break;
				}
				++uvIt;

			}
			return uvName;
		} else {
			return uvName;
		}
	}

	float generateBoneWeightForVertex(	const LightWave::Bone* bone, 
										const float inWeight, 
										const Math::Vector3& pt,
										const float fallExponent )
	{
		// no falloff for weightmap only
		if( bone->weightMapOnly == true )
			return inWeight;

		Math::Vector3 pt0 = bone->worldRestPos;
		Math::Vector3 pt1 = bone->worldRestPos + (bone->worldRestDir * bone->restLength);
		Math::LineSegment3 lineSegment(pt0, pt1);
		float dist = lineSegment.Distance( pt );
	
		float weight;
		weight = inWeight * (bone->strength / (powf(dist, fallExponent) + 1.0E-6f) );

		if(bone->limitedRange)
		{
			if( dist >= bone->outerLimit )
				weight = 0.f;
			else if( dist < bone->innerLimit )
			{
				float t = 1.0f - (dist / bone->innerLimit);
				weight = inWeight * bone->strength;
				weight *= 1.0E6f + 1.0E9f * t; //t is meant to be smooth(t);
			}
		}

		return weight;
	}

	void AddLWOLayers(	const LightWave::Object& in, 
						MeshMod::MeshPtr out, 
						const unsigned int layer, 
						const Math::Matrix4x4& transform )
	{
		std::vector<LightWave::Layer>::iterator layIt = in.loader->layers.begin();
		while( layIt != in.loader->layers.end() )
		{
			// skip if the layer is hidden
//			if( (*layIt).m_flags == LightWave::Layer::HIDDEN )
//			{
//				++layIt;
//				continue;
//			}
			// if we are only converting 1 layer skip if different
			if( layer != -1 && layer != (*layIt).number  )
			{
				++layIt;
				continue;
			}


			Math::Vector3 pivot( 0,0,0 );
			std::vector<LightWave::Layer>::iterator layItCur = layIt;

			while( (*layItCur).parent != LightWave::Layer::NO_PARENT )
			{
				std::vector<LightWave::Layer>::iterator layIt0 = in.loader->layers.begin();
				while( layIt0 != in.loader->layers.end() )
				{
					if( (*layIt0).number == (*layItCur).parent )
					{
						pivot += (*layIt0).pivot;
						layItCur = layIt0;
						break;
					}
					++layIt0;
				}
			}
			pivot += (*layItCur).pivot;

			const unsigned int baseVertex = out->getVertexContainer().size();
			const unsigned int baseFace = out->getFaceContainer().size();

			AddLWLayerVertices( *layIt, out, transform, pivot );
			
			AddLWLayerPolys( *layIt, out, baseVertex );

			AddLWLayerPolyTags( *layIt, out, baseFace );

			AddLWLayerVMap( *layIt, out, baseVertex );

			AddLWLayerDVMap( *layIt, out, baseVertex, baseFace );

			AddLWOBones( in, out, baseVertex );

			++layIt;
		}
	}

	void AddLWLayerVertices( const LightWave::Layer& in, MeshMod::MeshPtr out, const Math::Matrix4x4& transform, const Math::Vector3& pivot )
	{
		std::vector<LightWave::VEC12>::const_iterator ptIt = in.points.begin();
		while( ptIt != in.points.end() )
		{
			Math::Vector3 pt = Math::Vector3( (*ptIt)[0] - pivot[0], (*ptIt)[1] - pivot[1], (*ptIt)[2] - pivot[2]);
			pt = Math::TransformAndProject( pt, transform );

			out->addPosition( pt[0], pt[1], pt[2] );

			++ptIt;
		}
	}

	void AddLWLayerPolys( const LightWave::Layer& in, MeshMod::MeshPtr out, const unsigned int baseVertex )
	{
		std::vector<LightWave::Face>::const_iterator faceIt = in.faces.begin();
		while( faceIt != in.faces.end() ) {
			out->addPolygon( (*faceIt).verts, baseVertex);
			++faceIt;
		}
	}

	void AddLWLayerVMap( const LightWave::Layer& in, MeshMod::MeshPtr out, const unsigned int baseVertex )
	{
		// were we going to add the vmap elements
		MeshMod::VertexElementsContainer& vertCon = out->getVertexContainer();
		MeshMod::PointRepVertexElements* prEle = vertCon.getElements<MeshMod::PointRepVertexElements>();

		std::vector<LightWave::VertexMap*>::const_iterator vmIt = in.vertexMaps.begin();
		while( vmIt != in.vertexMaps.end() )
		{
			switch( (*vmIt)->getType() )
			{
			case LightWave::VertexMap::WGHT:
				{
					// new weight map always and set sub name to VMAP name
					MeshMod::Float1TupleVertexElements* wEle = 
						vertCon.addElements<MeshMod::Float1TupleVertexElements>( std::string("WGHT_") + (*vmIt)->name ); 

					// iterator across uv coords
					LightWave::WeightMap* wMap = (LightWave::WeightMap*) (*vmIt);
					std::vector<LightWave::WeightMap::Weight>::iterator wIt = wMap->weights.begin();
					while( wIt != wMap->weights.end() ) {
						const unsigned int startNum = wIt->index + baseVertex; 
						unsigned int vertNum = startNum;
						// add to vertex and all similar vertices
						do
						{
							float weight = wIt->value;
							(*wEle)[ vertNum ].data = weight;
							vertNum = (*prEle)[vertNum].next;
						} while( vertNum != startNum );
						++wIt;
					}
				}
				break;
			case LightWave::VertexMap::TXUV:
				{
					// new uv map always
					MeshMod::UVVertexElements* uvEle = 
						vertCon.addElements<MeshMod::UVVertexElements>( std::string("TXUV_") + (*vmIt)->name );

					// iterator across uv coords
					LightWave::UVMap* uvMap = (LightWave::UVMap*) (*vmIt);
					std::vector<LightWave::UVMap::UV>::iterator uvIt = uvMap->uvs.begin();
					while( uvIt != uvMap->uvs.end() ) {
						const unsigned int startNum = uvIt->index + baseVertex; 
						unsigned int vertNum = startNum;
						// add to vertex and all similar vertices
						do {
							(*uvEle)[ vertNum ].u = uvIt->u;
							(*uvEle)[ vertNum ].v = 1 - uvIt->v;
							vertNum = (*prEle)[vertNum].next;
						} while( vertNum != startNum );

						++uvIt;
					}
				}
				break;
			case LightWave::VertexMap::RGB:
			case LightWave::VertexMap::RGBA:
				break;
			case LightWave::VertexMap::MNVW:
				break;
			case LightWave::VertexMap::MORF:
				{
					// new uv map always
					MeshMod::PositionDeltaVertexElements* morfEle = 
						vertCon.addElements<MeshMod::PositionDeltaVertexElements>( std::string("MORF_") + (*vmIt)->name );

					// iterator across morf coords
					LightWave::MorfMap* morfMap = (LightWave::MorfMap*) (*vmIt);
					std::vector<LightWave::MorfMap::VertexDelta>::iterator morfIt = morfMap->deltas.begin();
					while( morfIt != morfMap->deltas.end() ) {
						const unsigned int startNum = morfIt->index + baseVertex; 
						unsigned int vertNum = startNum;
						// add to vertex and all similar vertices
						do {
							(*morfEle)[ vertNum ].dx = morfIt->x;
							(*morfEle)[ vertNum ].dy = morfIt->y;
							(*morfEle)[ vertNum ].dz = morfIt->z;
							vertNum = (*prEle)[vertNum].next;
						} while( vertNum != startNum );

						++morfIt;
					}
				}
				break;

			case LightWave::VertexMap::SPOT:
			case LightWave::VertexMap::PICK:
			default:
				break;
			}
			++vmIt;
		}
	}

	VertexIndex SplitVertexForFace( 	MeshMod::MeshPtr mesh,
			                        	MeshMod::VertexElementsContainer& vertCon,
	                               		MeshMod::PointRepVertexElements* prEle,
		                                MeshMod::HalfEdgeEdgeElements* edgeEle,
	                                	const VertexIndex oldVert, 
	                                	FaceIndex faceIdx ) {
		// point to the specified poly to this particular position+uv combo
		EdgeIndexContainer edgeList;
		mesh->getFaceEdges( faceIdx, edgeList );
	
		VertexIndex newVert = vertCon.cloneElement( oldVert );
		(*prEle)[ newVert ].next = (*prEle)[ oldVert ].next;
		(*prEle)[ oldVert ].next = newVert;

		// point edges to the new vertex with this poly uv's
		EdgeIndexContainer::const_iterator edIt = edgeList.cbegin();
		while( edIt != edgeList.cend() ) {
			if( (*edgeEle)[ (*edIt) ].startVertexIndex == oldVert )
				(*edgeEle)[ (*edIt) ].startVertexIndex = newVert;

			if( (*edgeEle)[ (*edIt) ].endVertexIndex == oldVert )
				(*edgeEle)[ (*edIt) ].endVertexIndex = newVert;

			++edIt;
		}	
		return newVert;
	}


	void AddLWLayerDVMap( const LightWave::Layer& in, MeshMod::MeshPtr out, 
							const unsigned int baseVertex,
							const unsigned int baseFace )
	{
		// were we going to add the vmap elements
		MeshMod::VertexElementsContainer& vertCon = out->getVertexContainer();
		MeshMod::EdgeElementsContainer& edgeCon = out->getEdgeContainer();
		
		MeshMod::PointRepVertexElements* prEle = vertCon.getElements<MeshMod::PointRepVertexElements>();
		MeshMod::HalfEdgeEdgeElements* edgeEle = edgeCon.getElements<MeshMod::HalfEdgeEdgeElements>();

		std::vector<LightWave::DiscVertexMap*>::const_iterator vmIt = in.discVertexMaps.begin();
		while( vmIt != in.discVertexMaps.end() ) {
			switch( (*vmIt)->getType() ) {
			case LightWave::VertexMap::WGHT:
				{
					const std::string subName = std::string("WGHT_") + (*vmIt)->name; // set sub name to WGHT name

					MeshMod::Float1TupleVertexElements* floatEle = vertCon.getOrAddElements<MeshMod::Float1TupleVertexElements>( subName );

					// iterator across weights
					LightWave::WeightDMap* wMap = (LightWave::WeightDMap*) (*vmIt);
					std::vector<LightWave::WeightDMap::Weight>::iterator wIt = wMap->weights.begin();
					while( wIt != wMap->weights.end() ) { 
						const VertexIndex oldVert = wIt->vIndex + baseVertex;
						const FaceIndex faceIdx = wIt->pIndex + baseFace;
	
						// TODO float1 markers to avoid unnessecary first split :(
						VertexIndex newVert = SplitVertexForFace( out, vertCon, prEle, edgeEle, oldVert, faceIdx );

						// update w coordinate
						(*floatEle)[ newVert ].data = wIt->value;
						++wIt;
					}
				}
				break;
			case LightWave::VertexMap::TXUV:
				{
					const std::string subName = std::string("TXUV_") + (*vmIt)->name; // set sub name to VMAP name

					MeshMod::UVVertexElements* uvEle = vertCon.getOrAddElements<MeshMod::UVVertexElements>( subName );

					// iterator across uv coords
					LightWave::UVDMap* uvMap = (LightWave::UVDMap*) (*vmIt);
					std::vector<LightWave::UVDMap::UV>::const_iterator uvIt = uvMap->uvs.cbegin();
					while( uvIt != uvMap->uvs.cend() ) {
						const unsigned int oldVert = uvIt->vIndex + baseVertex;
						const unsigned int faceIdx = uvIt->pIndex + baseFace;

						VertexIndex newVert;
						if( (*uvEle)[ newVert ].isValid() == false ) {
							// first vertex is still virgin so we can use it without cloning
							newVert = oldVert;
						} else {
							newVert = SplitVertexForFace( out, vertCon, prEle, edgeEle, oldVert, faceIdx );
						}

						// update u v coordinate
						(*uvEle)[ newVert ].u = uvIt->u;
						(*uvEle)[ newVert ].v = 1 - uvIt->v;

						++uvIt;
					}
				}
				break;
			case LightWave::VertexMap::RGB:
			case LightWave::VertexMap::RGBA:
				break;
			case LightWave::VertexMap::MNVW:
				break;
			case LightWave::VertexMap::MORF:
				{
					const std::string subName = std::string("MORF_") + (*vmIt)->name; // set sub name to MORF name

					MeshMod::PositionDeltaVertexElements* morfEle = vertCon.getOrAddElements<MeshMod::PositionDeltaVertexElements>( subName );

					// iterator across uv coords
					LightWave::MorfDMap* morfMap = (LightWave::MorfDMap*) (*vmIt);
					std::vector<LightWave::MorfDMap::VertexDelta>::iterator morfIt = morfMap->deltas.begin();
					while( morfIt != morfMap->deltas.end() ) { 
						const unsigned int oldVert = morfIt->vIndex + baseVertex;
						const unsigned int faceIdx = morfIt->pIndex + baseFace;

						// TODO morf markers to avoid unnessecary first split :(
						VertexIndex newVert = SplitVertexForFace( out, vertCon, prEle, edgeEle, oldVert, faceIdx );

						// update morph deltas
						(*morfEle)[ newVert ].dx = morfIt->x;
						(*morfEle)[ newVert ].dy = morfIt->y;
						(*morfEle)[ newVert ].dz = morfIt->z;
						++morfIt;
					}
				}
				break;
			case LightWave::VertexMap::SPOT:
			case LightWave::VertexMap::PICK:
			default:
				break;
			}
			++vmIt;
		}
	}

	// must be called before trianglation
	void AddLWLayerPolyTags( const LightWave::Layer& in, MeshMod::MeshPtr out, const unsigned int baseFace ) {
		// if we have some surface tag add them
		if( !in.surfaceTag2Poly.empty() ) {
			MeshMod::FaceElementsContainer& faceCon = out->getFaceContainer();

			if( faceCon.size() > 0) {
				// create surf poly tag face data if nessecary
				LWSurfPolyTagElements* surfTags = faceCon.getOrAddElements<LWSurfPolyTagElements>();

				// deposit surface tag in each polygon
				std::vector<LightWave::PolyTag>::const_iterator surfTagIt = in.surfaceTag2Poly.begin();
				while( surfTagIt != in.surfaceTag2Poly.end() ) {
					(*surfTags)[ (*surfTagIt).polygon + baseFace ].tagId = (*surfTagIt).tagID;
					++surfTagIt;
				}
			}
		}

		// if we have some surface tag add them
		if( !in.smgpTag2Poly.empty() ) {
			MeshMod::FaceElementsContainer& faceCon = out->getFaceContainer();

			if( faceCon.size() > 0) {
				// create smgp poly tag face data if nessecary
				LWSmgpPolyTagElements* smgpfTags = faceCon.getOrAddElements<LWSmgpPolyTagElements>();

				// deposit smoothing group tag in each polygon
				std::vector<LightWave::PolyTag>::const_iterator smgpTagIt = in.smgpTag2Poly.begin();
				while( smgpTagIt != in.smgpTag2Poly.end() ) {
					(*smgpfTags)[ (*smgpTagIt).polygon + baseFace].tagId = (*smgpTagIt).tagID;
					++smgpTagIt;
				}
			}
		}
	}

	void AddLWOClips( const LightWave::Object& in, MeshMod::MeshPtr out )
	{
		UNREFERENCED_PARAMETER( out );
		std::vector<LightWave::Clip>::iterator clipIt = in.loader->clip.begin();
		while( clipIt != in.loader->clip.end() )
		{
			// currently only support STIL
			if( (*clipIt).type == LightWave::Clip::STIL )
			{
				LWIdToFileName[ (*clipIt).id ] = (*clipIt).filename;
			} else
			{
				LWIdToFileName[ (*clipIt).id ] = "";
			}
			++clipIt;
		}
	}

	void AddLWOMaterials( const LightWave::Object& in, MeshMod::MeshPtr out ) {
		if( in.loader->surfaceMap.empty() )
			return;
		 
		MaterialElementsContainer& matCon = out->getMaterialContainer();

		// name elements
		NameMaterialElements* nameEle = matCon.getOrAddElements<NameMaterialElements>();

		//
		std::map<unsigned int, LightWave::Surface*>::iterator tagIt = in.loader->surfaceMap.begin();

		while( tagIt != in.loader->surfaceMap.end() ) {
			LightWave::Surface* curSurface = tagIt->second;

			const unsigned int matNum = matCon.size();
			matCon.resize( matNum + 1 );
			// fill in name
			(*nameEle)[ matNum ].matName = curSurface->name;

			// allow is to quickly jump from LW tag to material number in future
			LWSurfTagToMatNum[ curSurface->tagID ] = matNum;

			AddLWOSurface( curSurface, out );

			++tagIt;
		}
	}

	void ConvertSurfaceTagToMatNum( MeshMod::MeshPtr out ) {
		MeshMod::FaceElementsContainer& faceCon = out->getFaceContainer();

		// create surfacematerial array if nessary
		MaterialFaceElements* surfEle = faceCon.getOrAddElements<MaterialFaceElements>();
		LWSurfPolyTagElements* surfTags = faceCon.getElements<LWSurfPolyTagElements>();

		std::vector< FaceData::Material >::iterator surfIt = surfEle->elements.begin();

		while( surfIt != surfEle->elements.end() )
		{
			if( surfTags == 0 ) {
				// create an identity mat num array
				(*surfIt).surfaceIndex = 0;
			} else {
				const unsigned int index = (unsigned int) (surfIt - surfEle->elements.begin());

				(*surfIt).surfaceIndex = LWSurfTagToMatNum[ (*surfTags)[index].tagId ];
			}

			++surfIt;
		}
	}

	void AddLWOSurface( const LightWave::Surface* curSurface, MeshMod::MeshPtr out ) {
		MaterialElementsContainer& matCon = out->getMaterialContainer();

		const unsigned int matNum = LWSurfTagToMatNum[ curSurface->tagID ];

		// light params
		LightParamsMaterialElements* lightEle = matCon.getOrAddElements<LightParamsMaterialElements>();

		// Back face culling status
		BackFaceMaterialElements* backfaceEle = matCon.getOrAddElements<BackFaceMaterialElements>();

		// white base colour (if we have a texture else is LW colour
		float baseColour[3] = { 1,1,1 }; 
		// does this material have any colour blocks
		if(	!curSurface->colourBlocks.empty() ) {
			// TODO support all colour blocks
			TextureMaterialElements* textureEle = matCon.getOrAddElements<TextureMaterialElements>( "Colour" );

			// only support the first colour block 
			LightWave::LWBlok* curBlok = curSurface->colourBlocks[0];
			
			if( curBlok->type == LightWave::LWBlok::IMAP && curBlok->enable == true ) {
				const std::string fileName = LWIdToFileName[ curBlok->imageIndex ];
				(*textureEle)[matNum].fileName = fileName;
				(*textureEle)[matNum].uvChannel = generateUV( curBlok, out );
			}
		} else {
			baseColour[0] = curSurface->baseColour[0];
			baseColour[1] = curSurface->baseColour[1];
			baseColour[2] = curSurface->baseColour[2];
		}

		// fill in the lighting parameter
		(*lightEle)[ matNum ].diffuse[0] = curSurface->diffuse * baseColour[0];
		(*lightEle)[ matNum ].diffuse[1] = curSurface->diffuse * baseColour[1];
		(*lightEle)[ matNum ].diffuse[2] = curSurface->diffuse * baseColour[2];
		(*lightEle)[ matNum ].luminosity[0] = curSurface->luminosity * baseColour[0];
		(*lightEle)[ matNum ].luminosity[1] = curSurface->luminosity * baseColour[1];
		(*lightEle)[ matNum ].luminosity[2] = curSurface->luminosity * baseColour[2];
		// TODO colour highlight
		(*lightEle)[ matNum ].specular[0] = curSurface->specular;
		(*lightEle)[ matNum ].specular[1] = curSurface->specular;
		(*lightEle)[ matNum ].specular[2] = curSurface->specular;

		if( curSurface->specular > 0.f ) {
			(*lightEle)[ matNum ].specular_exponent = 
							(10.f * curSurface->glossiness) + 2.f;
		} else {
			(*lightEle)[ matNum ].specular_exponent = 0.f;
		}

		// back face culling
		(*backfaceEle)[ matNum ].facing = (curSurface->doubleSided ? 
						MaterialData::BackFace::NONE : MaterialData::BackFace::BACK);
	}

	/*

	for every bone
		for every vertex
			generateBoneWeightForVertex

	Lightwave has a case where if a vertex is not effected by any bone (can only happen with
	weight map only option) it uses the normal vertex transform. TODO fake this
	*/
	void AddLWOBones(const LightWave::Object& object, MeshMod::MeshPtr out, const unsigned int baseVertex ) {
		if( object.bones.empty() )
			return;

		const float falloffExp = object.getExponent(object.fallOffType);

		MeshMod::VertexElementsContainer& vertCon = out->getVertexContainer();

		BoneWeightsVertexElements* boneEle = vertCon.getOrAddElements<BoneWeightsVertexElements>();
		PositionVertexElements* posEle = vertCon.getElements<PositionVertexElements>();

		// for every bone in the object
		std::vector<LightWave::Bone*>::const_iterator boneIt = object.bones.begin();
		while( boneIt != object.bones.end() ) {
			const unsigned int boneNum = (unsigned int)(boneIt - object.bones.begin() );

			if( (*boneIt)->active == false )
				continue;

			if( (*boneIt)->weightMapName.empty() ) {
				// every vertex is effected by this bone (starts a base vertex)
				const unsigned int startVertex = baseVertex;
				const unsigned int endVertex = (unsigned int) boneEle->elements.size();
				for( unsigned int i = startVertex; i < endVertex; i++) {
					Math::Vector3 pt = Math::Vector3( (*posEle)[i].x,(*posEle)[i].y,(*posEle)[i].z );
					float weight = generateBoneWeightForVertex( (*boneIt), 1.f, pt, falloffExp );
					(*boneEle)[i].boneData.push_back( 
						MeshMod::VertexData::BoneWeights::Weight( boneNum, weight) );
				}
			} else {
				Float1TupleVertexElements* flEle = vertCon.getElements<Float1TupleVertexElements>( 
														std::string("WGHT_") + (*boneIt)->weightMapName );
				// every vertex is effected by this bone (starts a base vertex)
				// the weight will be zero for those no in the weight map
				const VertexIndex startVertex = baseVertex;
				const VertexIndex endVertex = (VertexIndex) boneEle->elements.size();
				for( VertexIndex i = startVertex; i < endVertex; i++) {
					Math::Vector3 pt = Math::Vector3( (*posEle)[i].x,(*posEle)[i].y,(*posEle)[i].z );
					float weight = generateBoneWeightForVertex( (*boneIt), (*flEle)[i].data, pt, falloffExp );
					(*boneEle)[i].boneData.push_back( 
						MeshMod::VertexData::BoneWeights::Weight( boneNum, weight) );
				}
			}
			++boneIt;
		}

		// normalise weights
		const unsigned int startVertex = baseVertex;
		const unsigned int endVertex = (unsigned int) boneEle->elements.size();
		for( unsigned int i = startVertex; i < endVertex; i++) {
			std::vector<MeshMod::VertexData::BoneWeights::Weight>::iterator wtIt;

			// sum weights
			float weightSum = 0.f;
			wtIt = (*boneEle)[i].boneData.begin();
			while( wtIt != (*boneEle)[i].boneData.end() ) {
				weightSum += (*wtIt).weight;
				++wtIt;
			}

			// approx zero divide check
			if( weightSum > 1e-4f ) {
				wtIt = (*boneEle)[i].boneData.begin();
				while( wtIt != (*boneEle)[i].boneData.end() ) {
					(*wtIt).weight /= weightSum;
					++wtIt;
				}
			}

			// sort weights by weight
			std::sort( (*boneEle)[i].boneData.begin(), (*boneEle)[i].boneData.end(), BoneWeightSort );
		}

	}
}

namespace MeshImport
{
	LWOImp::LWOImp( const std::string& filename ) {
		LightWave::Object object;
		LightWave::LWO_Loader loader( filename );
		object.loader = &loader;

		scene = std::make_shared<MeshMod::Scene>();
		mesh = LightWaveObject2GoMesh( object );
		MeshMod::SceneNodePtr node = std::make_shared<MeshMod::SceneNode>();
		node->type = "Mesh";
		node->name = filename;
		node->addObject( mesh );
		scene->sceneNodes.push_back( node );
	}
	bool LWOImp::loadedOk() const {
		if( scene && !scene->sceneNodes.empty() ) {
			return true;
		} else {
			return false;
		}
	}
	LWOImp::~LWOImp() {
	}
	MeshMod::ScenePtr LWOImp::toMeshMod() {
		return scene;
	}

}
/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
