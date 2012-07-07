/** \file Envelope.cpp
   LW Envelope's.
   LW Envelope evaluator 
   (c) 2001 Dean Calver
 */

//---------------------------------------------------------------------------
// Local Defines
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "lightwaveloader.h"
#include <stdlib.h>
#include <math.h>
#include "LWEnvelope.h"

//---------------------------------------------------------------------------
// Local Enums and typedefs
//---------------------------------------------------------------------------
using namespace LightWave;

//---------------------------------------------------------------------------
// Local Classes and structures
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Local Prototypes
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Global Variables
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Code
//---------------------------------------------------------------------------

/*
======================================================================
range()

Given the value v of a periodic function, returns the equivalent value
v2 in the principal interval [lo, hi].  If i isn't NULL, it receives
the number of wavelengths between v and v2.

   v2 = v - i * (hi - lo)

For example, range( 3 pi, 0, 2 pi, i ) returns pi, with i = 1.
====================================================================== */

static float range( float v, float lo, float hi, int *i )
{
   float v2, r = hi - lo;

   if ( r == 0.0 ) {
      if ( i ) *i = 0;
      return lo;
   }

   v2 = lo + v - r * ( float ) floor(( double ) v / r );
   if ( i ) *i = -( int )(( v2 - v ) / r + ( v2 > v ? 0.5 : -0.5 ));

   return v2;
}


/*
======================================================================
hermite()

Calculate the Hermite coefficients.
====================================================================== */

static void hermite( float t, float *h1, float *h2, float *h3, float *h4 )
{
   float t2, t3;

   t2 = t * t;
   t3 = t * t2;

   *h2 = 3.0f * t2 - t3 - t3;
   *h1 = 1.0f - *h2;
   *h4 = t3 - t2;
   *h3 = *h4 - t2 + t;
}


/*
======================================================================
bezier()

Interpolate the value of a 1D Bezier curve.
====================================================================== */

static float bezier( float x0, float x1, float x2, float x3, float t )
{
   float a, b, c, t2, t3;

   t2 = t * t;
   t3 = t2 * t;

   c = 3.0f * ( x1 - x0 );
   b = 3.0f * ( x2 - x1 ) - c;
   a = x3 - x0 - c - b;

   return a * t3 + b * t2 + c * t + x0;
}


/*
======================================================================
bez2_time()

Find the t for which bezier() returns the input time.  The handle
endpoints of a BEZ2 curve represent the control points, and these have
(time, value) coordinates, so time is used as both a coordinate and a
parameter for this curve type.
====================================================================== */

static float bez2_time( float x0, float x1, float x2, float x3, float time,
   float *t0, float *t1 )
{
   float v, t;

   t = *t0 + ( *t1 - *t0 ) * 0.5f;
   v = bezier( x0, x1, x2, x3, t );
   if ( fabs( time - v ) > .0001f ) {
      if ( v > time )
         *t1 = t;
      else
         *t0 = t;
      return bez2_time( x0, x1, x2, x3, time, t0, t1 );
   }
   else
      return t;
}


/*
======================================================================
bez2()

Interpolate the value of a BEZ2 curve.
====================================================================== */

float Envelope::bez2( const unsigned int key0, const unsigned int key1, float time ) const
{
   float x, y, t, t0 = 0.0f, t1 = 1.0f;

   if ( keys[key0].spantype == Envelope::Key::BEZIER2 )
      x = keys[key0].time + keys[key0].p2;
   else
      x = keys[key0].time + ( keys[key1].time - keys[key0].time ) / 3.0f;

   t = bez2_time( keys[key0].time, x, keys[key1].time + keys[key1].p0, 
					keys[key1].time, time, &t0, &t1 );

   if ( keys[key0].spantype == Envelope::Key::BEZIER2 )
      y = keys[key0].value + keys[key0].p3;
   else
      y = keys[key0].value + keys[key0].p1 / 3.0f;

   return bezier( keys[key0].value, y, keys[key1].p1 + keys[key1].value, 
						keys[key1].value, t );
}


/*
======================================================================
outgoing()

Return the outgoing tangent to the curve at key0.  The value returned
for the BEZ2 case is used when extrapolating a linear pre behavior and
when interpolating a non-BEZ2 span.
====================================================================== */

float Envelope::outgoing( const unsigned int key0, const unsigned int key1 ) const
{
   float a, b, d, t, out;

   switch ( keys[key1].spantype )
   {
   case LightWave::Envelope::Key::TCB:
			a =   ( 1.0f - keys[key1].tension )
				* ( 1.0f + keys[key1].continuity )
				* ( 1.0f + keys[key1].bias );
			b =   ( 1.0f - keys[key1].tension )
				* ( 1.0f - keys[key1].continuity )
				* ( 1.0f - keys[key1].bias );
			d = keys[key1].value - keys[key0].value;
			if ( key0 > 0 ){
				t = ( keys[key1].time - keys[key0].time ) / 
							( keys[key1].time - keys[key0-1].time );
				out = t * ( a * ( keys[key0].value - keys[key0-1].value ) + b * d );
			} else
		        out = b * d;
			break;
   case LightWave::Envelope::Key::LINEAR:
			d = keys[key1].value - keys[key0].value;
			if ( key0 > 0 ) {
				t = ( keys[key1].time - keys[key0].time ) / 
					( keys[key1].time - keys[key0-1].time );
				out = t * ( keys[key0].value - keys[key0-1].value + d );
			}
			else
				out = d;
	         break;
	case LightWave::Envelope::Key::BEZIER1:
	case LightWave::Envelope::Key::HERMITE:
			out = keys[key0].p1;
			if ( key0 > 0 ) 
				out *= ( keys[key1].time - keys[key0].time ) / 
						( keys[key1].time - keys[key0-1].time );
			break;

	case LightWave::Envelope::Key::BEZIER2:
         out = keys[key0].p3 * ( keys[key1].time - keys[key0].time );
         if ( fabs( keys[key0].p2 ) > 1e-5f )
            out /= keys[key0].p2;
         else
            out *= 1e5f;
         break;

	case LightWave::Envelope::Key::STEPPED:
		default:
         out = 0.0f;
         break;
   }

   return out;
}


/*
======================================================================
incoming()

Return the incoming tangent to the curve at key1.  The value returned
for the BEZ2 case is used when extrapolating a linear post behavior.
====================================================================== */

float Envelope::incoming( const unsigned int key0, const unsigned int key1 ) const
{
   float a, b, d, t, in;

   switch ( keys[key1].spantype )
   {
	case LightWave::Envelope::Key::LINEAR:
			d = keys[key1].value - keys[key0].value;
			if ( key1 < keys.size()-1 ) {
				t = ( keys[key1].time - keys[key0].time ) / 
					( keys[key1+1].time - keys[key0].time );
				in = t * ( keys[key1+1].value - keys[key1].value + d );
			}
			else
				in = d;
			break;
	case LightWave::Envelope::Key::TCB:
			a =   ( 1.0f - keys[key1].tension )
				* ( 1.0f - keys[key1].continuity )
				* ( 1.0f + keys[key1].bias );
			b =   ( 1.0f - keys[key1].tension )
				* ( 1.0f + keys[key1].continuity )
				* ( 1.0f - keys[key1].bias );
			d = keys[key1].value - keys[key0].value;

			if ( key1 < keys.size()-1 ) {
				t = ( keys[key1].time - keys[key0].time ) / 
					( keys[key1+1].time - keys[key0].time );
				in = t * ( b * ( keys[key1+1].value - keys[key1].value ) + a * d );
			}
			else
		        in = a * d;
	    break;

	case LightWave::Envelope::Key::BEZIER1:
	case LightWave::Envelope::Key::HERMITE:
			in = keys[key1].p0;
			if ( key1 < keys.size()-1 ) 
				in *= ( keys[key1].time - keys[key0].time ) / 
						( keys[key1+1].time - keys[key0].time );
			break;

	case LightWave::Envelope::Key::BEZIER2:
         in = keys[key1].p1 * ( keys[key1].time - keys[key0].time );
         if ( fabs( keys[key1].p0 ) > 1e-5f )
            in /= keys[key1].p0;
         else
            in *= 1e5f;
         break;
	case LightWave::Envelope::Key::STEPPED:
	default:
         in = 0.0f;
         break;
   }

   return in;
}

bool Envelope::inPost(float time) const
{
   if ( numKeys <= 1 )
	   return true;
   if ( time > keys[keys.size()-1].time ) 
	   return true;
   else return false;
}

bool Envelope::inPre(float time) const
{
   if ( numKeys <= 1 )
	   return true;
   if ( time < keys[0].time ) 
	   return true;
   else return false;
}

/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/
float Envelope::evaluate( float time ) const
{
   float t, h1, h2, h3, h4, in, out, offset = 0.0f;
   int noff;

   /* if there's no key, the value is 0 */

   if ( numKeys == 0 )
	   return 0.0f;

   /* if there's only one key, the value is constant */
   if ( numKeys == 1 )
	   return keys.begin()->value;

   const unsigned int skey = 0; // first
   const unsigned int ekey = (unsigned int) keys.size()-1; // last

   /* use pre-behavior if time is before first key time */

   if ( time < keys[skey].time ) 
   {
       switch ( pre )
       {
		case LightWave::Envelope::RESET:
            return 0.0f;
		case LightWave::Envelope::CONSTANT:
            return keys[skey].value;
		case LightWave::Envelope::REPEAT:
            time = range( time, keys[skey].time, keys[ekey].time, NULL );
            break;
		case LightWave::Envelope::OSCILLATE:
            time = range( time, keys[skey].time, keys[ekey].time, &noff );
            if ( noff % 2 )
               time = keys[ekey].time - keys[skey].time - time;
            break;
		case LightWave::Envelope::OFFSET_REPEAT:
            time = range( time, keys[skey].time, keys[ekey].time, &noff );
            offset = noff * ( keys[ekey].value - keys[skey].value );
            break;
		case LightWave::Envelope::LINEAR:
			out = outgoing( skey, skey + 1 )
                / ( keys[skey + 1].time - keys[skey].time );
            return out * ( time - keys[skey].time ) + keys[skey].value;
      }
   }

   /* use post-behavior if time is after last key time */

   else if ( time > keys[ekey].time ) 
   {
       switch ( post )
       {
		case LightWave::Envelope::RESET:
            return 0.0f;
		case LightWave::Envelope::CONSTANT:
            return keys[ekey].value;
		case LightWave::Envelope::REPEAT:
            time = range( time, keys[skey].time, keys[ekey].time, NULL );
            break;
		case LightWave::Envelope::OSCILLATE:
            time = range( time, keys[skey].time, keys[ekey].time, &noff );
            if ( noff % 2 )
               time = keys[ekey].time - keys[skey].time - time;
            break;
		case LightWave::Envelope::OFFSET_REPEAT:
            time = range( time, keys[skey].time, keys[ekey].time, &noff );
            offset = noff * ( keys[ekey].value - keys[skey].value );
            break;

         case LightWave::Envelope::LINEAR:
            in = incoming( ekey-1, ekey )
               / ( keys[ekey].time - keys[ekey-1].time );
            return in * ( time - keys[ekey].time ) + keys[ekey].value;
      }
   }

   /* get the endpoints of the interval being evaluated */
   unsigned int key0;

   key0 = 0;
   while ( time > keys[key0+1].time )
	   key0++;

   unsigned int key1 = key0+1;

   /* check for singularities first */

   if ( time == keys[key0].time )
      return keys[key0].value + offset;
   else if ( time == keys[key1].time )
      return keys[key1].value + offset;

   /* get interval length, time in [0, 1] */

   t = ( time - keys[key0].time ) / ( keys[key1].time - keys[key0].time );

   /* interpolate */

   switch ( keys[key1].spantype )
   {
   case LightWave::Envelope::Key::TCB:
   case LightWave::Envelope::Key::HERMITE:
   case LightWave::Envelope::Key::BEZIER1:
         out = outgoing( key0, key1 );
         in = incoming( key0, key1 );
         hermite( t, &h1, &h2, &h3, &h4 );
         return h1 * keys[key0].value + h2 * keys[key1].value + h3 * out + h4 * in + offset;
   case LightWave::Envelope::Key::BEZIER2:
         return bez2( key0, key1, time ) + offset;
   case LightWave::Envelope::Key::LINEAR:
         return keys[key0].value + t * ( keys[key1].value - keys[key0].value ) + offset;
   case LightWave::Envelope::Key::STEPPED:
         return keys[key0].value + offset;
      default:
         return offset;
   }
}
/**
Short description.
Detailed description
@param param description
@return description
@exception description
*/

