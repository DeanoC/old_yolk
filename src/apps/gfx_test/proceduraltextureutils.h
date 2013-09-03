#pragma once
#ifndef YOLK_PROCEDURALTEXTUREUTILS_H_
#define YOLK_PROCEDURALTEXTUREUTILS_H_

float Noise( const float x, const float y, const float z );
float Noise( const Math::Vector3& pt );
float FBm( const Math::Vector3& p, float omega, int octaves);
float FBm( const float x, const float y, const float z, float omega, int octaves);
float Turbulence( const float x, const float y, const float z, float omega, int octaves );
float Turbulence( const Math::Vector3& p, float omega, int octaves );

#endif
