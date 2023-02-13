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

#include "FrameGraphFrameResources.h"
#include "FrameGraphResourcePool.h"

#include <yave/graphics/commands/CmdBufferRecorder.h>
#include <yave/graphics/buffers/buffers.h>
#include <yave/graphics/device/DeviceProperties.h>

#include <y/utils/memory.h>


namespace yave {

FrameGraphFrameResources::FrameGraphFrameResources(std::shared_ptr<FrameGraphResourcePool> pool) : _pool(pool) {
}

FrameGraphFrameResources::~FrameGraphFrameResources() {
    Y_TODO(release images before the cmd buffer is recycled)
    for(auto&& res : _image_storage) {
        _pool->release(std::move(res));
    }
    for(auto&& res : _buffer_storage) {
        _pool->release(std::move(res));
    }
    _pool->garbage_collect();
}

u32 FrameGraphFrameResources::create_image_id() {
    return _next_image_id++;
}

u32 FrameGraphFrameResources::create_buffer_id() {
    return _next_buffer_id++;
}

void FrameGraphFrameResources::reserve(usize images, usize buffers) {
    _images.set_min_capacity(images);
    _buffers.set_min_capacity(buffers);
}

void FrameGraphFrameResources::init_staging_buffer() {
    if(_staging_buffer_len) {
        _staging_buffer = StagingBuffer(_staging_buffer_len);
    }
}

void FrameGraphFrameResources::create_image(FrameGraphImageId res, ImageFormat format, const math::Vec2ui& size, ImageUsage usage) {
    res.check_valid();

    _images.set_min_size(res.id() + 1);

    auto& image = _images[res.id()];
    y_always_assert(!image, "Image already exists");

    _image_storage.emplace_back(_pool->create_image(format, size, usage));
    image = &_image_storage.back();
}

void FrameGraphFrameResources::create_buffer(FrameGraphBufferId res, u64 byte_size, BufferUsage usage, MemoryType memory) {
    res.check_valid();

    _buffers.set_min_size(res.id() + 1);

    auto& buffer = _buffers[res.id()];
    y_always_assert(!buffer.buffer, "Buffer already exists");

    if(is_cpu_visible(memory)) {
        y_debug_assert(_staging_buffer.is_null());
        buffer.staging_buffer_offset = _staging_buffer_len;
        _staging_buffer_len += align_up_to(byte_size, device_properties().non_coherent_atom_size);

    }

    _buffer_storage.emplace_back(_pool->create_buffer(byte_size, usage, MemoryType::DeviceLocal));
    buffer.buffer = &_buffer_storage.back();
}

void FrameGraphFrameResources::flush_mapped_buffers(CmdBufferRecorder& recorder) {
    y_profile();
    const auto region = recorder.region("Flush buffers");

    for(const auto& buffer : _buffers) {
        if(buffer.buffer && buffer.is_mapped()) {
            recorder.copy(staging_buffer(buffer), TransientSubBuffer<BufferUsage::TransferDstBit>(*buffer.buffer) );
        }
    }
}

bool FrameGraphFrameResources::is_alive(FrameGraphImageId res) const {
    return res.id() < _images.size() && _images[res.id()] != nullptr;
}

bool FrameGraphFrameResources::is_alive(FrameGraphBufferId res) const {
    return res.id() < _buffers.size() && _buffers[res.id()].buffer != nullptr;
}

ImageBarrier FrameGraphFrameResources::barrier(FrameGraphImageId res, PipelineStage src, PipelineStage dst) const {
    res.check_valid();
    return ImageBarrier(*_images[res.id()], src, dst);
}

BufferBarrier FrameGraphFrameResources::barrier(FrameGraphBufferId res, PipelineStage src, PipelineStage dst) const {
    res.check_valid();
    return BufferBarrier(*_buffers[res.id()].buffer, src, dst);
}

const ImageBase& FrameGraphFrameResources::image_base(FrameGraphImageId res) const {
    return find(res);
}

const BufferBase& FrameGraphFrameResources::buffer_base(FrameGraphBufferId res) const {
    return find(res);
}


void FrameGraphFrameResources::create_alias(FrameGraphImageId dst, FrameGraphImageId src) {
    dst.check_valid();
    src.check_valid();

    TransientImage<>* orig = _images[src.id()];
    y_always_assert(orig, "Source image doesn't exists");

    _images.set_min_size(dst.id() + 1);

    TransientImage<>*& image = _images[dst.id()];
    y_always_assert(!image, "Destination image already exists");

    image = orig;
}

bool FrameGraphFrameResources::are_aliased(FrameGraphImageId a, FrameGraphImageId b) const {
    return &find(a) == &find(b);
}







const TransientImage<>& FrameGraphFrameResources::find(FrameGraphImageId res) const {
    y_always_assert(res.is_valid(), "Invalid image resource");

    if(!is_alive(res)) {
        y_fatal("Image resource doesn't exist");
    }

    return *_images[res.id()];
}

const TransientBuffer& FrameGraphFrameResources::find(FrameGraphBufferId res) const {
    y_always_assert(res.is_valid(), "Invalid buffer resource");

    if(!is_alive(res)) {
        y_fatal("Buffer resource doesn't exist");
    }

    return *_buffers[res.id()].buffer;
}

StagingSubBuffer FrameGraphFrameResources::staging_buffer(FrameGraphMutableBufferId res) const {
    y_always_assert(res.is_valid(), "Invalid buffer resource");
    y_always_assert(!_staging_buffer.is_null(), "Staging buffer has not been initialized");

    if(!is_alive(res)) {
        y_fatal("Buffer resource doesn't exist");
    }

    const auto buffer = _buffers[res.id()];
    if(!buffer.is_mapped()) {
        y_fatal("Buffer has not been mapped");
    }

    return staging_buffer(buffer);
}


StagingSubBuffer FrameGraphFrameResources::staging_buffer(const BufferData& buffer) const {
    y_debug_assert(buffer.is_mapped());
    return StagingSubBuffer(_staging_buffer, buffer.buffer->byte_size(), buffer.staging_buffer_offset);
}


}

