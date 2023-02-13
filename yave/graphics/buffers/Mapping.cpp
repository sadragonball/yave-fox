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

#include "Mapping.h"
#include "buffers.h"

#include <yave/graphics/graphics.h>
#include <yave/graphics/commands/CmdBufferRecorder.h>

namespace yave {

Mapping::Mapping(const SubBuffer<BufferUsage::None, MemoryType::CpuVisible>& buffer) :
        _buffer(buffer),
        _mapping(static_cast<u8*>(_buffer.device_memory().map()) + buffer.byte_offset()) {

    y_debug_assert(_buffer.byte_offset() % _buffer.host_side_alignment() == 0);
    y_debug_assert(_mapping);
}

Mapping::~Mapping() {
    flush();
}

void Mapping::flush() {
    if(_mapping) {
        const VkMappedMemoryRange range = _buffer.vk_memory_range();
        Y_TODO(Maybe merge flush & unmap)
        vk_check(vkFlushMappedMemoryRanges(vk_device(), 1, &range));
        _buffer.device_memory().unmap();
    }
}

usize Mapping::byte_size() const {
    return _buffer.byte_size();
}

void Mapping::swap(Mapping& other) {
    std::swap(_mapping, other._mapping);
    std::swap(_buffer, other._buffer);
}

void* Mapping::data() {
    return _mapping;
}

const void* Mapping::data() const {
    return _mapping;
}

void Mapping::stage(const SubBuffer<BufferUsage::TransferDstBit>& dst, CmdBufferRecorder& recorder, const void* data, usize elem_size, usize input_stride) {
    const u64 dst_size = dst.byte_size();

    const StagingBuffer buffer(dst_size);
    Mapping map(buffer);
    if(!data) {
        std::memset(map.data(), 0, dst_size);
    } else {
        if(!elem_size) {
            std::memcpy(map.data(), data, dst_size);
        } else {
            input_stride = std::max(input_stride, elem_size);
            const usize elem_count = dst_size / elem_size;

            u8* out_data = static_cast<u8*>(map.data());
            const u8* input_data = static_cast<const u8*>(data);
            for(usize i = 0; i != elem_count; ++i) {
                std::memcpy(out_data, input_data, elem_size);
                out_data += elem_size;
                input_data += input_stride;
            }
        }
    }
    recorder.copy(buffer, dst);
}

}

