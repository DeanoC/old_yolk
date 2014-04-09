// Borrowed from Freetype-GL and seperated into its own file.
// Original header below
/* =========================================================================
* Freetype GL - A C OpenGL Freetype engine
* Platform:    Any
* WWW:         http://code.google.com/p/freetype-gl/
* -------------------------------------------------------------------------
* Copyright 2011 Nicolas P. Rougier. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY NICOLAS P. ROUGIER ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL NICOLAS P. ROUGIER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Nicolas P. Rougier.
* ========================================================================= */
#include <cmath>
#include <cstring>
#include <algorithm>

// ------------------------------------------------------ MitchellNetravali ---
// Mitchell Netravali reconstruction filter
float MitchellNetravali(float x) {
	const float B = 1 / 3.0f, C = 1 / 3.0f; // Recommended
	// const float B =   1.0, C =   0.0; // Cubic B-spline (smoother results)
	// const float B =   0.0, C = 1/2.0; // Catmull-Rom spline (sharper results)
	x = fabs(x);
	if (x < 1)
		return ((12 - 9 * B - 6 * C) * x * x * x
		+ (-18 + 12 * B + 6 * C) * x * x
		+ (6 - 2 * B)) / 6;
	else if (x < 2)
		return ((-B - 6 * C) * x * x * x
		+ (6 * B + 30 * C) * x * x
		+ (-12 * B - 48 * C) * x
		+ (8 * B + 24 * C)) / 6;
	else
		return 0;
}

// ------------------------------------------------------------ interpolate ---
float hq_interpolate(float x, float y0, float y1, float y2, float y3) {
	using namespace std;

	float c0 = MitchellNetravali(x - 1);
	float c1 = MitchellNetravali(x);
	float c2 = MitchellNetravali(x + 1);
	float c3 = MitchellNetravali(x + 2);
	float r = c0*y0 + c1*y1 + c2*y2 + c3*y3;
	return min(max(r, 0.0f), 1.0f);
}


// ------------------------------------------------------------------ scale ---
int hq_resample(	float *src_data, int src_width, int src_height,
					float *dst_data, int dst_width, int dst_height) {
	using namespace std;

	if ((src_width == dst_width) && (src_height == dst_height))
	{
		memcpy(dst_data, src_data, src_width*src_height*sizeof(float));
		return 0;
	}
	size_t i, j;
	float xscale = src_width / (float)dst_width;
	float yscale = src_height / (float)dst_height;
	for (j = 0; j < dst_height; ++j)
	{
		for (i = 0; i < dst_width; ++i)
		{
			int src_i = (int)floor(i * xscale);
			int src_j = (int)floor(j * yscale);
			int i0 = min(max(0, src_i - 1), src_width - 1);
			int i1 = min(max(0, src_i), src_width - 1);
			int i2 = min(max(0, src_i + 1), src_width - 1);
			int i3 = min(max(0, src_i + 2), src_width - 1);
			int j0 = min(max(0, src_j - 1), src_height - 1);
			int j1 = min(max(0, src_j), src_height - 1);
			int j2 = min(max(0, src_j + 1), src_height - 1);
			int j3 = min(max(0, src_j + 2), src_height - 1);
			float t0 = hq_interpolate(i / (float)dst_width,
				src_data[j0*src_width + i0],
				src_data[j0*src_width + i1],
				src_data[j0*src_width + i2],
				src_data[j0*src_width + i3]);
			float t1 = hq_interpolate(i / (float)dst_width,
				src_data[j1*src_width + i0],
				src_data[j1*src_width + i1],
				src_data[j1*src_width + i2],
				src_data[j1*src_width + i3]);
			float t2 = hq_interpolate(i / (float)dst_width,
				src_data[j2*src_width + i0],
				src_data[j2*src_width + i1],
				src_data[j2*src_width + i2],
				src_data[j2*src_width + i3]);
			float t3 = hq_interpolate(i / (float)dst_width,
				src_data[j3*src_width + i0],
				src_data[j3*src_width + i1],
				src_data[j3*src_width + i2],
				src_data[j3*src_width + i3]);
			float y = hq_interpolate(j / (float)dst_height, t0, t1, t2, t3);
			dst_data[j*dst_width + i] = y;
		}
	}
	return 0;
}