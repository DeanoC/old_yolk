#pragma once
#ifndef YOLK_CONVERT_HQ_RESAMPLER_H_
#define YOLK_CONVERT_HQ_RESAMPLER_H_ 1

float hq_interpolate(float x, float y0, float y1, float y2, float y3);
int hq_resample(float *src_data, int src_width, int src_height,
				float *dst_data, int dst_width, int dst_height);
float MitchellNetravali(float x);

#endif