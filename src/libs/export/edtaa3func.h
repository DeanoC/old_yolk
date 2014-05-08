#pragma once
#ifndef YOLK_CONVERT_EDTAA3FUNC_H_
#define YOLK_CONVERT_EDTAA3FUNC_H_ 1

typedef float edtaa3real;

void computegradient(const edtaa3real *img, int w, int h, edtaa3real *gx, edtaa3real *gy);
void edtaa3(const edtaa3real *img, const edtaa3real *gx, const edtaa3real *gy, int w, int h, short *distx, short *disty, edtaa3real *dist);


#endif