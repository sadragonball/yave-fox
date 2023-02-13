/*******************************
Copyright (c) 2016-2023 Grégoire Angerand

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

#include "FrameGraphResourcePool.h"

#include <y/utils/log.h>
#include <y/utils/format.h>

namespace yave {

template<typename U>
static void check_usage(U u) {
    if(u == U::None) {
        y_fatal("Invalid resource usage.");
    }
}

FrameGraphResourcePool::FrameGraphResourcePool() {
}

FrameGraphResourcePool::~FrameGraphResourcePool() {
    const auto image_lock = y_profile_unique_lock(_image_lock);
    const auto buffer_lock = y_profile_unique_lock(_buffer_lock);
}

TransientImage<> FrameGraphResourcePool::create_image(ImageFormat format, const math::Vec2ui& size, ImageUsage usage) {
    y_profile();

    check_usage(usage);

    TransientImage<> image;
    if(create_image_from_pool(image, format, size, usage)) {
        return image;
    }

    y_profile_zone("create image");
    return TransientImage<>(format, usage, size);
}

TransientBuffer FrameGraphResourcePool::create_buffer(u64 byte_size, BufferUsage usage, MemoryType memory) {
    y_profile();

    check_usage(usage);

    if(memory == MemoryType::DontCare) {
        memory = prefered_memory_type(usage);
    }

    TransientBuffer buffer;
    if(create_buffer_from_pool(buffer, byte_size, usage, memory)) {
        return buffer;
    }

    y_profile_zone("create buffer");
    return TransientBuffer(byte_size, usage, memory);
}

bool FrameGraphResourcePool::create_image_from_pool(TransientImage<>& res, ImageFormat format, const math::Vec2ui& size, ImageUsage usage) {
    const auto lock = y_profile_unique_lock(_image_lock);

    for(auto it = _images.begin(); it != _images.end(); ++it) {
        auto& img = it->first;

        if(img.format() == format && img.size() == size && img.usage() == usage) {

            res = std::move(img);
            _images.erase(it);

            y_debug_assert(!res.is_null());

            return true;
        }
    }
    return false;
}

bool FrameGraphResourcePool::create_buffer_from_pool(TransientBuffer& res, usize byte_size, BufferUsage usage, MemoryType memory) {
    const auto lock = y_profile_unique_lock(_buffer_lock);

    for(auto it = _buffers.begin(); it != _buffers.end(); ++it) {
        auto& buffer = it->first;

        if(buffer.byte_size() == byte_size &&
           buffer.usage() == usage &&
           (buffer.memory_type() == memory || memory == MemoryType::DontCare)) {

            res = std::move(buffer);
            _buffers.erase(it);

            y_debug_assert(!res.is_null());

            return true;
        }
    }
    return false;
}


void FrameGraphResourcePool::release(TransientImage<> image) {
    const auto lock = y_profile_unique_lock(_image_lock);

    y_debug_assert(!image.is_null());
    _images.emplace_back(std::move(image), _collection_id);
}

void FrameGraphResourcePool::release(TransientBuffer buffer) {
    const auto lock = y_profile_unique_lock(_buffer_lock);

    y_debug_assert(!buffer.is_null());
    _buffers.emplace_back(std::move(buffer), _collection_id);
}

void FrameGraphResourcePool::garbage_collect() {
    y_profile();

    const u64 max_col_count = 6;
    const u64 collect_id = _collection_id++;

    {
        const auto lock = y_profile_unique_lock(_image_lock);
        for(usize i = 0; i < _images.size(); ++i) {
            if(_images[i].second + max_col_count < collect_id) {
                _images.erase(_images.begin() + i);
                --i;
            }
        }
    }

    {
        const auto lock = y_profile_unique_lock(_buffer_lock);
        for(usize i = 0; i < _buffers.size(); ++i) {
            if(_buffers[i].second + max_col_count < collect_id) {
                _buffers.erase(_buffers.begin() + i);
                --i;
            }
        }
    }
}


}

