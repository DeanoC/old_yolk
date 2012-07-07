#include "lightwaveloader.h"

#include "LWSceneLoader.h"
#include "DynamicsItemPlugin.h"

namespace
{

using namespace LightWave;
//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------


#define SK &SceneLoader::skipKey

SceneLoader::KEY_DATA DIP_KeyArray[] = {
	// Objects
	{ "DynamicsItem",				SK,	SK,	&SceneLoader::DynamicsItemSubKey },
	{ "Version",					SK,	SK,	&SceneLoader::DynamicsItemVersionSubKey },
	{ "Type",						SK,	SK,	&SceneLoader::DynamicsItemTypeSubKey },
	{ "Enabled",					SK,	SK,	&SceneLoader::DynamicsItemEnabledSubKey },
	{ "Shape",						SK,	SK,	&SceneLoader::DynamicsItemShapeSubKey },
	{ "InitialActivation",			SK,	SK,	&SceneLoader::DynamicsItemInitialActivationSubKey },
	{ "ManualActivation",			SK,	SK,	&SceneLoader::DynamicsItemManualActivationSubKey },
	{ "DeactivationTime",			SK,	SK,	&SceneLoader::DynamicsItemDeactivationTimeSubKey },
	{ "DeactivationLinearSpeed",	SK,	SK,	&SceneLoader::DynamicsItemDeactivationLinearSpeedSubKey },
	{ "DeactivationAngularSpeed",	SK,	SK,	&SceneLoader::DynamicsItemDeactivationAngularSpeedSubKey },
	{ "CollisionMargin",			SK,	SK,	&SceneLoader::DynamicsItemCollisionMarginSubKey },
	{ "MassMethod",					SK,	SK,	&SceneLoader::DynamicsItemMassMethodSubKey },
	{ "GivenMass",					SK,	SK,	&SceneLoader::DynamicsItemGivenMassSubKey },
	{ "Density",					SK,	SK,	&SceneLoader::DynamicsItemDensitySubKey },
	{ "Friction",					SK,	SK,	&SceneLoader::DynamicsItemFrictionSubKey },
	{ "Restitution",				SK,	SK,	&SceneLoader::DynamicsItemRestitutionSubKey },
	{ "LinearDamping",				SK,	SK,	&SceneLoader::DynamicsItemLinearDampingSubKey },
	{ "AngularDamping",				SK,	SK,	&SceneLoader::DynamicsItemAngularDampingSubKey },
	{ "GlueStrength",				SK,	SK,	&SceneLoader::DynamicsItemGlueStrengthSubKey },
	{ "BreakingAngle",				SK,	SK,	&SceneLoader::DynamicsItemBreakingAngleSubKey },
	{ "EnableBreakingAngle",		SK,	SK,	&SceneLoader::DynamicsItemEnableBreakingAngleSubKey },
	{ "BreakingDistance",			SK,	SK,	&SceneLoader::DynamicsItemBreakingDistanceSubKey },
	{ "EnableBreakingDistance",		SK,	SK,	&SceneLoader::DynamicsItemEnableBreakingDistanceSubKey },
	{ "MergePoints",				SK,	SK,	&SceneLoader::DynamicsItemMergePointsSubKey },
	{ "SoftSolverMethod",			SK,	SK,	&SceneLoader::DynamicsItemSoftSolverMethodSubKey },
	{ "LinearStiffness",			SK,	SK,	&SceneLoader::DynamicsItemLinearStiffnessSubKey },
	{ "PoseMatching",				SK,	SK,	&SceneLoader::DynamicsItemPoseMatchingSubKey },
	{ "PoseVolumeRatio",			SK,	SK,	&SceneLoader::DynamicsItemPoseVolumeRatioSubKey },
	{ "DampingCoefficient",			SK,	SK,	&SceneLoader::DynamicsItemDampingCoefficientSubKey },
	{ "DragCoefficient",			SK,	SK,	&SceneLoader::DynamicsItemDragCoefficientSubKey },
	{ "PressureCoefficient",		SK,	SK,	&SceneLoader::DynamicsItemPressureCoefficientSubKey },
	{ "VolumeConservation",			SK,	SK,	&SceneLoader::DynamicsItemVolumeConservationSubKey },
	{ "DynamicFrictionCoefficient",	SK,	SK,	&SceneLoader::DynamicsItemDynamicFrictionCoefficientSubKey },
	{ "RigidContactHardness",		SK,	SK,	&SceneLoader::DynamicsItemRigidContactHardnessSubKey },
	{ "KinematicContactHardness",	SK,	SK,	&SceneLoader::DynamicsItemKinematicContactHardnessSubKey },
	{ "SoftContactHardness",		SK,	SK,	&SceneLoader::DynamicsItemSoftContactHardnessSubKey },
	{ "NumberOfClusters",			SK,	SK,	&SceneLoader::DynamicsItemNumberOfClustersSubKey },
	{ "RigidCollisionMethod",		SK,	SK,	&SceneLoader::DynamicsItemRigidCollisionMethodSubKey },
	{ "SoftCollisionMethod",		SK,	SK,	&SceneLoader::DynamicsItemSoftCollisionMethodSubKey },
	{ "SelfCollision",				SK,	SK,	&SceneLoader::DynamicsItemSelfCollisionSubKey },
	{ "VelocitySolverIterations",	SK,	SK,	&SceneLoader::DynamicsItemVelocitySolverIterationsSubKey },
	{ "PositionSolverIterations",	SK,	SK,	&SceneLoader::DynamicsItemPositionSolverIterationsSubKey },
	{ "DriftSolverIterations",		SK,	SK,	&SceneLoader::DynamicsItemDriftSolverIterationsSubKey },
	{ "ClusterSolverIterations",	SK,	SK,	&SceneLoader::DynamicsItemClusterSolverIterationsSubKey },

	// end of key array
	{ "",0,	0, 0 } 
};

SceneLoader::KEY_DATA DIP_EndKeyArray[] = {
	{ "EndPlugin",			SK,	SK,	&SceneLoader::EndPluginKey },
	{ "", 0, 0, 0 } 
};

}

namespace LightWave {

const std::string DynamicsItemPlugin::PluginName = "DynamicsItem";

void SceneLoader::DynamicsItemPluginCallback( FILE* f ) {
	subKeyReader( f, DIP_KeyArray, DIP_EndKeyArray );
}

void SceneLoader::DynamicsItemSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin == 0 );
	assert( curObject != 0 );
	curNodePlugin =  new DynamicsItemPlugin();
	curObject->plugins.push_back( curNodePlugin );
}

void SceneLoader::DynamicsItemVersionSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->version = x;
}


void SceneLoader::DynamicsItemTypeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	char* val0  = strtok(const_cast<char*>(ValueText)," \xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->type = x;
}

void SceneLoader::DynamicsItemEnabledSubKey( FILE* f, const char* ValueText ) {
	
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->enabled = x;
}

void SceneLoader::DynamicsItemShapeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->shape = x;
}

void SceneLoader::DynamicsItemInitialActivationSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->initialActivation = x;
}

void SceneLoader::DynamicsItemManualActivationSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->manualActivation = x;
}

void SceneLoader::DynamicsItemDeactivationTimeSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->deactivationTime = x;
}
void SceneLoader::DynamicsItemDeactivationLinearSpeedSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->deactivationLinearSpeed = x;
}
void SceneLoader::DynamicsItemDeactivationAngularSpeedSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->deactivationAngularSpeed = x;
}

void SceneLoader::DynamicsItemCollisionMarginSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->collisionMargin = x;
}

void SceneLoader::DynamicsItemMassMethodSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->massMethod = x;
}

void SceneLoader::DynamicsItemGivenMassSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->givenMass = x;
}

void SceneLoader::DynamicsItemDensitySubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->density = x;
}

void SceneLoader::DynamicsItemFrictionSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->friction = x;
}

void SceneLoader::DynamicsItemRestitutionSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->restitution = x;
}

void SceneLoader::DynamicsItemLinearDampingSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->linearDamping = x;
}

void SceneLoader::DynamicsItemAngularDampingSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->angularDamping = x;
}

void SceneLoader::DynamicsItemGlueStrengthSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->glueStrength = x;
}

void SceneLoader::DynamicsItemBreakingAngleSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->breakingAngle = x;
}

void SceneLoader::DynamicsItemEnableBreakingAngleSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->enableBreakingAngle = x;
}

void SceneLoader::DynamicsItemBreakingDistanceSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->breakingDistance = x;
}

void SceneLoader::DynamicsItemEnableBreakingDistanceSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->enableBreakingDistance = x;
}

void SceneLoader::DynamicsItemMergePointsSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->mergePoints = x;
}

void SceneLoader::DynamicsItemSoftSolverMethodSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->softSolverMethod = x;
}

void SceneLoader::DynamicsItemLinearStiffnessSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->linearStiffness = x;
}

void SceneLoader::DynamicsItemPoseMatchingSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->poseMatching = x;
}

void SceneLoader::DynamicsItemPoseVolumeRatioSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->poseVolumeRatio = x;
}

void SceneLoader::DynamicsItemDampingCoefficientSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->dampingCoefficient = x;
}

void SceneLoader::DynamicsItemDragCoefficientSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->dragCoefficient = x;
}

void SceneLoader::DynamicsItemLiftCoefficientSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->liftCoefficient = x;
}

void SceneLoader::DynamicsItemPressureCoefficientSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->pressureCoefficient = x;
}

void SceneLoader::DynamicsItemVolumeConservationSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->volumeConservation = x;
}

void SceneLoader::DynamicsItemDynamicFrictionCoefficientSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->dynamicFrictionCoefficient = x;
}

void SceneLoader::DynamicsItemRigidContactHardnessSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->rigidContactHardness = x;
}

void SceneLoader::DynamicsItemKinematicContactHardnessSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->kinematicContactHardness = x;
}

void SceneLoader::DynamicsItemSoftContactHardnessSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->softContactHardness = x;
}

void SceneLoader::DynamicsItemAnchorHardnessSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	float x = (float) atof( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->anchorHardness = x;
}

void SceneLoader::DynamicsItemNumberOfClustersSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->numberOfClusters = x;
}

void SceneLoader::DynamicsItemRigidCollisionMethodSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->rigidCollisionMethod = x;
}

void SceneLoader::DynamicsItemSoftCollisionMethodSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->softCollisionMethod = x;
}

void SceneLoader::DynamicsItemSelfCollisionSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	bool x = !!atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->selfCollision = x;
}

void SceneLoader::DynamicsItemVelocitySolverIterationsSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->velocitySolverIterations = x;
}

void SceneLoader::DynamicsItemPositionSolverIterationsSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->positionSolverIterations = x;
}

void SceneLoader::DynamicsItemDriftSolverIterationsSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->driftSolverIterations = x;
}

void SceneLoader::DynamicsItemClusterSolverIterationsSubKey( FILE* f, const char* ValueText ) {
	assert( curNodePlugin != 0 );
	assert( curNodePlugin->pluginName == "DynamicsItem" );

	readLine(f);
	char* val0 = strtok(lineBuffer," \t\xA\xD");
	unsigned int x = (unsigned int) atoi( val0 );

	((DynamicsItemPlugin*)curNodePlugin)->clusterSolverIterations = x;
}

}