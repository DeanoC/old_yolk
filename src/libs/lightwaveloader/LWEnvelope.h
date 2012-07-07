/** \file envelope.h
   header for lightwave envelope's (animation system).
   Handles LW animation envelopes
   (c) 2001 Dean Calver
 */

#if !defined( LIGHTWAVE_ENVELOPE_H_ )
#define LIGHTWAVE_ENVELOPE_H_

#if _MSC_VER > 1000
#pragma once
#endif

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Enums and typedefs
//---------------------------------------------------------------------------
namespace LightWave
{

	//---------------------------------------------------------------------------
	// Classes and structures
	//---------------------------------------------------------------------------

	/**
	A Lightwave envelope.
	A lightwave envelope is a complex animation path, with a 
	potentally varing algorithm per key
	*/
	class Envelope {
	public:
		/**
		Each key (segment) of the spline/envelope.
		*/
		struct Key {
			enum SPAN_TYPE {
				TCB				=	0,
				HERMITE			=	1,
				BEZIER1			=	2,
				LINEAR			=	3,
				STEPPED			=	4,
				BEZIER2			=	5,
			};

			float value;
			float time;
			SPAN_TYPE spantype;
			float tension;
			float continuity;
			float bias;
			float p0;
			float p1;
			float p2;
			float p3;
		};

		// public acces to data members
		unsigned int		numKeys;
		std::vector<Key>	keys;

		enum BEHAVIOUR_TYPE {
			RESET			=	0,
			CONSTANT		=	1,
			REPEAT			=	2,
			OSCILLATE		=	3,
			OFFSET_REPEAT	=	4,
			LINEAR			=	5,
		};
		BEHAVIOUR_TYPE		pre;
		BEHAVIOUR_TYPE		post;

	private:
		float bez2( const unsigned int key0, const unsigned int key1, float time ) const;
		float incoming( const unsigned int key0, 
						const unsigned int key1 ) const;
		float outgoing( const unsigned int key0, 
						const unsigned int key1 ) const;
	public:
		/// evaluate envelope at time
		float evaluate( float time ) const;
		bool inPost( float time ) const;
		bool inPre( float time ) const;
	};
}
#endif

