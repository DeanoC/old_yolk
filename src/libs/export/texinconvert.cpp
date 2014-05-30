
#include "core/core.h"
#include "scene/generictextureformat.h"
#include "scene/gtfcracker.h"
#include "textureimage.h"
#include "texexport.h"

namespace Export {

	union uConv {
		double		d;
		uint64_t	i64;
		struct {
			float		f[2];
		};
		struct {
			uint32_t	i[2];
		};
		struct {
			half_float::half h[4];
		};
		struct {
			uint16_t s[4];
		};
		struct {
			uint8_t b[8];
		};

	};
	
	void convertInputToTextureImage(Export::BitmapInput& in) {

		using namespace Export;

		bool isInFloat = ((in.flags & 
			(BitmapInput::BI_HALF | BitmapInput::BI_FLOAT | BitmapInput::BI_DOUBLE)) != 0);
		unsigned int inBits =
			(in.flags & BitmapInput::BI_DOUBLE) ? 64 :
			(in.flags & (BitmapInput::BI_UINT32 | BitmapInput::BI_FLOAT)) ? 32 :
			(in.flags & (BitmapInput::BI_UINT16 | BitmapInput::BI_HALF)) ? 16 :
			8;
		const uint8_t* data = in.data;

		CORE_ASSERT(in.channels < 16);

		TextureImage<double> outTexture(in.channels, in.width, in.height);
		auto outData = outTexture.getData().begin();

		// expand integer data to high precision
		uConv expd[16]; // 16 channels at stupid precision... should do for now

		for (auto y = 0; y < in.height; ++y) {
			for (auto x = 0; x < in.width; ++x) {

				// make channel mismatch obvious
				memset(expd, 0xFF, sizeof(double)* 16);

				// expand inputs to a 32 bit binary representation
				for (uint32_t c = 0; c < in.channels; ++c) {
					// repeat load N times for later masking
					switch (inBits) {
					case 64: expd[c].i64 = *(uint64_t*)(data); data += 8; break;
					case 32: for (int i = 0; i < 2; ++i){ expd[c].i[i] = *(uint32_t*)(data); }; data += 4; break;
					case 16: for (int i = 0; i < 4; ++i){ expd[c].s[i] = *(uint16_t*)(data); };  data += 2; break;
					case 8:  for (int i = 0; i < 8; ++i){ expd[c].b[i] = *(uint8_t*)(data); }; data += 1; break;
					default: break;
					}
				}

				// TODO generalised input swizzle 
				// for 4 channel format (RGBA or ARGB) we swap to our prefered RGBA format
				if (in.channels == 4) {
					// output is currently always LE RGBA (TODO endianess correctness)
					CORE_ASSERT(in.channels == 4);
					if (in.flags & BitmapInput::BI_RGBA) {
						// in rgba = out abgr (AKA LE RGBA)
						const uConv r = expd[0];
						const uConv g = expd[1];
						const uConv b = expd[2];
						const uConv a = expd[3];
						expd[0] = a;
						expd[1] = b;
						expd[2] = g;
						expd[3] = r;
					}
					else {
						// in argb = out abgr
						const uConv a = expd[0];
						const uConv r = expd[1];
						const uConv g = expd[2];
						const uConv b = expd[3];
						expd[0] = a;
						expd[1] = b;
						expd[2] = g;
						expd[3] = r;
					}
				}

				// format conversion, handle different bit width, float/integer, normalisation etc.
				for (unsigned int c = 0; c < in.channels; ++c) {
					// changing float bit depth portion 
					if (isInFloat) {
						if (inBits == 16) {
							// half load from binary
							expd[c].d = expd[c].h[0]; // half to double convert
						}
						else {
							if (inBits == 32) {
								// float load from binary
								expd[c].d = expd[c].f[0];
							}
							// double don't need any convertion
						}
					}
					else {
						// int to float
						expd[c].d = (double)(expd[c].i64 & ((1LL << inBits) - 1LL));
						if (BitmapInput::BI_NORMALISED) {
							expd[c].d /= ((1 << inBits)-1); // the classic edge case..
						}
					}

					// store into texture image
					outData[c] = expd[c].d;
				}
				outData += in.channels;
			}
		}

		in.textureImage = std::make_shared<TextureImage<double>>(outTexture);
	}

} // end namespace