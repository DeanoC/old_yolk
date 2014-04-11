#pragma once
#ifndef YOLK_CONVERT_EDTAA3FUNC_H_
#define YOLK_CONVERT_EDTAA3FUNC_H_ 1

typedef float edtaa3real;

void prepBinaryImage(const uint8_t* in, const int width, const int height, edtaa3real* img);
void computegradient(edtaa3real *img, int w, int h, edtaa3real *gx, edtaa3real *gy);
void edtaa3(edtaa3real *img, edtaa3real *gx, edtaa3real *gy, int w, int h, short *distx, short *disty, edtaa3real *dist);


#endif