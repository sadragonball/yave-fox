/*******************************
Copyright (c) 2016-2017 Grégoire Angerand

This code is licensed under the MIT License (MIT).

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "ImageFormat.h"

namespace yave {

ImageFormat::ImageFormat(vk::Format format) : _format(format) {
}

vk::Format ImageFormat::vk_format() const {
	return _format;
}

usize ImageFormat::bpp() const {
	switch(_format) {
		case vk::Format::eR8G8B8A8Unorm:
			return 4;
		case vk::Format::eD32Sfloat:
			return 4;

		default:
			return fatal("Unsupported image format");
	}
}

vk::ImageAspectFlags ImageFormat::vk_aspect() const {
	switch(_format) {
		case vk::Format::eR8G8B8A8Unorm:
			return vk::ImageAspectFlagBits::eColor;

		case vk::Format::eD32Sfloat:
			return vk::ImageAspectFlagBits::eDepth;

		default:
			return fatal("Unsupported image format");
	}
}


}
