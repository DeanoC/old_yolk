#pragma once
#ifndef YOLK_CONVERT_EDTAA3FUNC_H_
#define YOLK_CONVERT_EDTAA3FUNC_H_ 1

void prepBinaryImage(const uint8_t* in, const int width, const int height, float* img);
void computegradient(float *img, int w, int h, float *gx, float *gy);
void edtaa3(float *img, float *gx, float *gy, int w, int h, short *distx, short *disty, float *dist);


#endif