#pragma once
#ifndef YOLK_CONVERT_HQ_RESAMPLER_H_
#define YOLK_CONVERT_HQ_RESAMPLER_H_ 1
template<typename real = float>
real MitchellNetravali(const real x, const real _b = real(1) / real(3), const real _c = real(1) / real(3));

template<typename real = float>
real hq_interpolate(real x, real y0, real y1, real y2, real y3, const real _b = real(1) / real(3), const real _c = real(1) / real(3));

template<typename real = float>
void hq_resample(const unsigned int _channelCount, 
				const real *_srcData,	const unsigned int _srcWidth, const unsigned int _srcHeight,
				real *_dstData,			const unsigned int _dstWidth, const unsigned int _dstHeight, 
				const real _b = real(1) / real(3), const real _c = real(1) / real(3),
				const real _clampLow = real(0), const real _clampHigh = real(1));



#include "hq_resampler.inl"


#endif