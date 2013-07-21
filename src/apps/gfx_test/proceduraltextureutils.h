#pragma once
#ifndef YOLK_PROCEDURALTEXTUREUTILS_H_
#define YOLK_PROCEDURALTEXTUREUTILS_H_

float Noise( const Math::Vector3& pt );
float FBm( const Math::Vector3& p, float omega, int octaves);
float Turbulence( const Math::Vector3& p, float omega, int octaves );

#endif
