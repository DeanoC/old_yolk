/** \file DynamicsMasterPlugin.h
   (c) 2012 Dean Calver
 */

#if !defined( LIGHTWAVE_DYNAMICS_ITEM_PLUGIN_H_ )
#define LIGHTWAVE_DYNAMICS_ITEM_PLUGIN_H_

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "LWNodePlugin.h"

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{

//---------------------------------------------------------------------------
// Classes and structures
//---------------------------------------------------------------------------
	/**
	Short description.
	Detailed description
	*/
	struct DynamicsItemPlugin : public LightWave::NodePlugin {

		unsigned int 	version;
		unsigned int 	type;
		bool 			enabled;
		unsigned int 	shape;
		bool 			initialActivation;
		bool 			manualActivation;
		float 			deactivationTime;
		float 			deactivationLinearSpeed;
		float 			deactivationAngularSpeed;
		float 			collisionMargin;
		unsigned int 	massMethod;			
		float 			givenMass;				
		float 			density;				
		float 			friction;
		float 			restitution;
		float 			linearDamping;
		float 			angularDamping;
		float 			glueStrength;
		float 			breakingAngle;
		bool 			enableBreakingAngle;
		float 			breakingDistance;
		bool 			enableBreakingDistance;
		bool 			mergePoints;
		unsigned int 	softSolverMethod;
		float 			linearStiffness;
		bool 			poseMatching;		
		float 			poseVolumeRatio;
		float 			dampingCoefficient;
		float 			dragCoefficient;
		float 			liftCoefficient;
		float 			pressureCoefficient;
		bool 			volumeConservation;
		float 			dynamicFrictionCoefficient;
		float 			rigidContactHardness;
		float 			kinematicContactHardness;
		float 			softContactHardness;
		float 			anchorHardness;
		unsigned int 	numberOfClusters;
		unsigned int 	rigidCollisionMethod;
		unsigned int 	softCollisionMethod;
		bool 			selfCollision;
		unsigned int 	velocitySolverIterations;
		unsigned int 	positionSolverIterations;
		unsigned int 	driftSolverIterations;
		unsigned int 	clusterSolverIterations;

		static const std::string PluginName;

		DynamicsItemPlugin() : 
			NodePlugin( PluginName ),
			version( 0 ),
			enabled( false ),
			shape( 0 ),
			initialActivation( false ),
			manualActivation( false ),
			deactivationTime( 1.0f ),
			deactivationLinearSpeed( 0.1f ),
			deactivationAngularSpeed( 0.08726f ),
			collisionMargin( 0.04f ),
			massMethod( 1 ),
			givenMass( 1.0f ),
			density( 1000.0f ),
			friction( 0.5f ),
			restitution( 0.3f ),
			linearDamping( 0.1f ),
			angularDamping( 0.01f ),
			glueStrength( 0.1f ),
			breakingAngle( 0.8726f ),
			enableBreakingAngle( false ),
			breakingDistance( 0.5f ),
			enableBreakingDistance( false ),
			mergePoints( false ),
			softSolverMethod( 0 ),
			linearStiffness( 0.1f ),
			poseMatching( false ),
			poseVolumeRatio( 1.0f ),
			dampingCoefficient( 0.0f ),
			dragCoefficient( 0.0f ),
			liftCoefficient( 0.0f ),
			pressureCoefficient( 0.0f ),
			volumeConservation( false ),
			dynamicFrictionCoefficient( 0.2f ),
			rigidContactHardness( 1.0f ),
			kinematicContactHardness( 1.0f ),
			softContactHardness( 1.0f ),
			anchorHardness( 1.0f ),
			numberOfClusters( 8 ),
			rigidCollisionMethod( 0 ),
			softCollisionMethod( 0 ),
			selfCollision( false ),
			velocitySolverIterations( 6 ),
			positionSolverIterations( 6 ),
			driftSolverIterations( 6 ),
			clusterSolverIterations( 6 )
		{
		}
	};	

} // end namespace
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
