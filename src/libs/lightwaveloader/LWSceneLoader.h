/** \file LWLayer.h
   A Lightwave object V2 layer.
   Also contains geometry etc
   (c) 2002 Dean Calver
 */

#if !defined( LIGHTWAVE_SCENELOADER_H_ )
#define LIGHTWAVE_SCENELOADER_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWNode.h"
#include "LWObject.h"
#include "LWBone.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{
	class NodePlugin;
	class LWO_Loader;

	/**
	object representing a light wave scene.
	Detailed description
	*/
	class SceneLoader
	{
	protected:
	public:
		SceneLoader();
		~SceneLoader();

		void loadFile( const std::string& filename );

		typedef std::vector<Object*> ObjectList;
		typedef std::vector<Bone*> BoneList;
		typedef std::vector<NodePlugin*> PluginDataList;
		ObjectList			objects;
		BoneList			bones;
		PluginDataList		masterPluginData;

		struct KEY_DATA {
			const char* text;
			void (SceneLoader::*FuncV1)(FILE *f, const char* value );
			void (SceneLoader::*FuncV3)(FILE *f, const char* value );
			void (SceneLoader::*FuncV5)(FILE *f, const char* value );
		};

	protected:
		// plugin system
		typedef void (SceneLoader::*PluginCallback)( FILE *f );
		void registerMasterPlugin( const std::string& name, PluginCallback pPlugin );
		void registerItemMotionPlugin( const std::string& name, PluginCallback pPlugin );
		void registerCustomObjPlugin( const std::string& name, PluginCallback pPlugin );

		void keyReader( FILE *f );
		void subKeyReader(FILE *f, const KEY_DATA* subKeyArray, const KEY_DATA* exitKeyArray);

		int findKey(const std::string& text, const KEY_DATA* keyArray);

		void readLine( FILE* f );
		char* skipWhiteSpace();

		void linkNodes();

		unsigned int		version;
		unsigned int		lineNum;
		char				lineBuffer[255];
		bool				ungetLastLine;
		char				lastLineBuffer[255];
		std::unordered_map< std::string, LightWave::LWO_Loader*> lwoLoaderCache;

		Bone*				curBone;
		Object*				curObject;
		NodePlugin*			curNodePlugin;
		ChannelGroup*		curChannelGroup;
		unsigned int		curChannelCount;

		typedef std::map<std::string, PluginCallback > PlugInMap;
		PlugInMap masterPluginRegistry;
		PlugInMap itemMotionPluginRegistry;
		PlugInMap customObjectPluginRegistry;
	
	public:
		// keys
		void skipKey(FILE* f, const char* ValueText);
		void LWSCKey(FILE* f, const char* ValueText);
		void LoadObjectLayerKey(FILE* f, const char* ValueText);
		void LoadObjectLayerKeyV5(FILE* f, const char* ValueText);
		void AddNullObjectKey(FILE* f, const char* ValueText);
		void AddNullObjectKeyV5(FILE* f, const char* ValueText);
		void AddBoneKey(FILE* f, const char* ValueText);
		void AddBoneKeyV5(FILE* f, const char* ValueText);

		// sub keys
		void ObjectMotionKey(FILE* f, const char* ValueText);
		void BoneMotionKey(FILE* f, const char* ValueText);
		void NumChannelsKey(FILE* f, const char* ValueText);
		void ChannelKey(FILE* f, const char* ValueText);
		void ParentItemKey(FILE* f, const char* ValueText);
		void PluginKey(FILE* f, const char* ValueText);
		void EndPluginKey(FILE* f, const char* ValueText);
		
		void BoneNameKey(FILE* f, const char* ValueText);
		void BoneActiveKey(FILE* f, const char* ValueText);
		void BoneRestPositionKey(FILE* f, const char* ValueText);
		void BoneRestDirectionKey(FILE* f, const char* ValueText);
		void BoneRestLengthKey(FILE* f, const char* ValueText);
		void BoneWeightMapNameKey(FILE* f, const char* ValueText);
		void BoneWeightMapOnlyKey(FILE* f, const char* ValueText);
		void BoneNormalizationKey(FILE* f, const char* ValueText);
		void BoneStrengthKey(FILE* f, const char* ValueText);
		void BoneScaleBoneStrengthKey(FILE* f, const char* ValueText);
		void BoneFalloffTypeKey(FILE* f, const char* ValueText);


		// plugins handlers
		// dynamic master plugin (per scene)
		void DynamicsMasterPluginCallback( FILE* f );
		void DynamicsMasterSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterVersionSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterDrawShapesSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterEnabledSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterStepsPerSecondSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterTimeScaleSubKey( FILE* f, const char* ValueText );
		void DynamicsMasterGravitySubKey( FILE* f, const char* ValueText );
		// dynamic item plugin (per node)
		void DynamicsItemPluginCallback( FILE* f );
		void DynamicsItemSubKey( FILE* f, const char* ValueText );
		void DynamicsItemVersionSubKey( FILE* f, const char* ValueText );
		void DynamicsItemTypeSubKey( FILE* f, const char* ValueText );
		void DynamicsItemEnabledSubKey( FILE* f, const char* ValueText );
		void DynamicsItemShapeSubKey( FILE* f, const char* ValueText );
		void DynamicsItemInitialActivationSubKey( FILE* f, const char* ValueText );
		void DynamicsItemManualActivationSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDeactivationTimeSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDeactivationLinearSpeedSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDeactivationAngularSpeedSubKey( FILE* f, const char* ValueText );
		void DynamicsItemCollisionMarginSubKey( FILE* f, const char* ValueText );
		void DynamicsItemMassMethodSubKey( FILE* f, const char* ValueText );
		void DynamicsItemGivenMassSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDensitySubKey( FILE* f, const char* ValueText );
		void DynamicsItemFrictionSubKey( FILE* f, const char* ValueText );
		void DynamicsItemRestitutionSubKey( FILE* f, const char* ValueText );
		void DynamicsItemLinearDampingSubKey( FILE* f, const char* ValueText );
		void DynamicsItemAngularDampingSubKey( FILE* f, const char* ValueText );
		void DynamicsItemGlueStrengthSubKey( FILE* f, const char* ValueText );
		void DynamicsItemBreakingAngleSubKey( FILE* f, const char* ValueText );
		void DynamicsItemEnableBreakingAngleSubKey( FILE* f, const char* ValueText );
		void DynamicsItemBreakingDistanceSubKey( FILE* f, const char* ValueText );
		void DynamicsItemEnableBreakingDistanceSubKey( FILE* f, const char* ValueText );
		void DynamicsItemMergePointsSubKey( FILE* f, const char* ValueText );
		void DynamicsItemSoftSolverMethodSubKey( FILE* f, const char* ValueText );
		void DynamicsItemLinearStiffnessSubKey( FILE* f, const char* ValueText );
		void DynamicsItemPoseMatchingSubKey( FILE* f, const char* ValueText );
		void DynamicsItemPoseVolumeRatioSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDampingCoefficientSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDragCoefficientSubKey( FILE* f, const char* ValueText );
		void DynamicsItemLiftCoefficientSubKey( FILE* f, const char* ValueText );
		void DynamicsItemPressureCoefficientSubKey( FILE* f, const char* ValueText );
		void DynamicsItemVolumeConservationSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDynamicFrictionCoefficientSubKey( FILE* f, const char* ValueText );
		void DynamicsItemRigidContactHardnessSubKey( FILE* f, const char* ValueText );
		void DynamicsItemKinematicContactHardnessSubKey( FILE* f, const char* ValueText );
		void DynamicsItemSoftContactHardnessSubKey( FILE* f, const char* ValueText );
		void DynamicsItemAnchorHardnessSubKey( FILE* f, const char* ValueText );
		void DynamicsItemNumberOfClustersSubKey( FILE* f, const char* ValueText );
		void DynamicsItemRigidCollisionMethodSubKey( FILE* f, const char* ValueText );
		void DynamicsItemSoftCollisionMethodSubKey( FILE* f, const char* ValueText );
		void DynamicsItemSelfCollisionSubKey( FILE* f, const char* ValueText );
		void DynamicsItemVelocitySolverIterationsSubKey( FILE* f, const char* ValueText );
		void DynamicsItemPositionSolverIterationsSubKey( FILE* f, const char* ValueText );
		void DynamicsItemDriftSolverIterationsSubKey( FILE* f, const char* ValueText );
		void DynamicsItemClusterSolverIterationsSubKey( FILE* f, const char* ValueText );
		
		// vtdfh (game) level plugin
		void VtdfhLevelPluginCallback( FILE* f );
		void VtdfhLevelNameSubKey( FILE* f, const char* ValueText );
		void VtdfhLevelDescSubKey( FILE* f, const char* ValueText );
		// vtdfh (game) enemy plugin
		void VtdfhEnemyPluginCallback( FILE* f );
		void VtdfhEnemySubKey( FILE* f, const char* ValueText );
		void VtdfhEnemyTypeSubKey( FILE* f, const char* ValueText );
		// vtdfh (game) marker plugin
		void VtdfhMarkerPluginCallback( FILE* f );
		void VtdfhMarkerSubKey( FILE* f, const char* ValueText );
		void VtdfhMarkerTypeSubKey( FILE* f, const char* ValueText );

	};
}
//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------

/**
Short description.
Detailed description
*/

//---------------------------------------------------------------------------
// Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Externals
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// End Header file
//---------------------------------------------------------------------------
#endif
