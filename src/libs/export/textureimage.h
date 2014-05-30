#pragma once
#ifndef YOLK_EXPORT_TEXTUREIMAGE_H_
#define YOLK_EXPORT_TEXTUREIMAGE_H_ 1

#include "export/hq_resampler.h"

namespace Export {

	template< typename real = float >
	class TextureImage {
	public:
		explicit TextureImage(const unsigned int _channelCount, const unsigned int _width, const unsigned int _height = 1,
			const unsigned int _depth = 1, const unsigned int _slices = 1);
		explicit TextureImage(const TextureImage<real>& _rhs);

		void setData(const real* _data);
		void setData(const std::vector<real>& _data);

		std::shared_ptr<TextureImage<real>> resizeFilter(const unsigned int _width, const unsigned int _height = 1,
			const unsigned int _depth = 1, const real _b = real(1) / real(3), const real _c = real(1) / real(3) ) const;
		std::shared_ptr<TextureImage<real>> changeChannelCount(unsigned int newChannelCount) const;

		unsigned int getChannelCount() const { return channelCount; }
		unsigned int getWidth() const { return width; }
		unsigned int getHeight() const { return height; }
		unsigned int getDepth() const { return depth; }
		unsigned int getSlices() const { return slices; }
		std::vector<real>& getData() { return data; }
		const std::vector<real>& getData() const { return data; }

		real value(const unsigned int _c, const unsigned int _x, const unsigned int _y = 0,
			const unsigned int _d = 0, const unsigned int _s = 0) const;

	protected:
		std::vector<real> data;

		unsigned int channelCount;
		unsigned int width;
		unsigned int height;
		unsigned int depth;
		unsigned int slices;

		unsigned int size;
	};

	template<typename real>
	TextureImage<real>::TextureImage(const unsigned int _channelCount,
		const unsigned int _width, const unsigned int _height,
		const unsigned int _depth, const unsigned int _slices) :
		channelCount(_channelCount), width(_width), height(_height), depth(_depth), slices(_slices),
		size(_channelCount * _width * _height * _depth * _slices),
		data(_channelCount * _width * _height * _depth * _slices, real(0)) {
	}

	template<typename real>
	TextureImage<real>::TextureImage(const TextureImage<real>& _rhs) :
		channelCount(_rhs.channelCount), width(_rhs.width), height(_rhs.height), 
		depth(_rhs.depth), slices(_rhs.slices),
		size(_rhs.channelCount * _rhs.width * _rhs.height * _rhs.depth * _rhs.slices)
	{
		data = _rhs.data;
	}

	template<typename real>
	void TextureImage<real>::setData(const real* _data) {
		std::copy(_data, _data + size, data.begin());
	}
	template<typename real>
	void TextureImage<real>::setData(const std::vector<real>& _data) {
		CORE_ASSERT(_data.size() == size);
		data = _data; // deep copy
	}

	template<typename real>
	std::shared_ptr<TextureImage<real>> TextureImage<real>::resizeFilter(
		const unsigned int _width, const unsigned int _height,
		const unsigned int _depth, const real _b, const real _c ) const {
		const unsigned int size3D(channelCount * width * height * depth);

		auto out = std::make_shared<TextureImage<real>>( channelCount, _width, _height, _depth, getSlices() );

		for (unsigned int i = 0; i < getSlices(); ++i){
			unsigned int spliceStartOffset = size3D*i;
			const auto& inDS = data.begin() + spliceStartOffset;
			auto& outDS = out->data.begin() + spliceStartOffset;
			// 1 or 2D textures in and out
			if (_depth == 1 && getDepth() == 1) {
				hq_resample<real>(channelCount, 
									&(*inDS), getWidth(), getHeight(),
									&(*outDS), out->getWidth(), out->getHeight(), _b, _c);
			}
			else {
				LOG(FATAL) << "3D textures not supported yet\n";
			}
		}
		return out;
	}
	template<typename real>
	std::shared_ptr<TextureImage<real>> TextureImage<real>::changeChannelCount(unsigned int newChannelCount) const {

		const unsigned int size3D(channelCount * width * height * depth);
		auto out = std::make_shared<TextureImage<real>>(newChannelCount, getWidth(), getHeight(), getDepth(), getSlices());

		const auto& inDS = data.begin();
		auto& outDS = out->data.begin();

		unsigned int j = 0;
		for (unsigned int i = 0; i < size; ++i){
			unsigned int chan = i % channelCount;
			if (chan < newChannelCount) {
				outDS[j++] = inDS[i];
			}
		}

		return out;
	}

	template<typename real>
	real TextureImage<real>::value(const unsigned int _c, const unsigned int _x, const unsigned int _y,
		const unsigned int _z, const unsigned int _s) const {
		CORE_ASSERT(_c < channelCount);
		CORE_ASSERT(_x < width);
		CORE_ASSERT(_y < height);
		CORE_ASSERT(_z < depth);
		CORE_ASSERT(_s < slices);

		const unsigned int size1D(channelCount * width);
		const unsigned int size2D(channelCount * width * height);
		const unsigned int size3D(channelCount * width * height * depth);
		auto index = (_s * size3D) + (_z * size2D) + (_y * size1D) + (_x * channelCount) + _c;

		return *(getData().begin() + index);
	}

} // end namespace 

#endif // endif include