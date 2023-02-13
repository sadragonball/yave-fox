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
#ifndef YAVE_GRAPHICS_BUFFERS_SUBBUFFER_H
#define YAVE_GRAPHICS_BUFFERS_SUBBUFFER_H

#include <yave/graphics/memory/DeviceMemoryView.h>

#include "Buffer.h"

namespace yave {

class SubBufferBase {

    public:
        SubBufferBase() = default;
        SubBufferBase(const BufferBase& base, u64 byte_len, u64 byte_off);
        SubBufferBase(const SubBufferBase& base, u64 byte_len, u64 byte_off);

        explicit SubBufferBase(const BufferBase& base);

        bool is_null() const;

        u64 byte_size() const;
        u64 byte_offset() const;

        VkBuffer vk_buffer() const;

        DeviceMemoryView device_memory() const;

        VkDescriptorBufferInfo descriptor_info() const;
        VkMappedMemoryRange vk_memory_range() const;

        bool operator==(const SubBufferBase& other) const;
        bool operator!=(const SubBufferBase& other) const;

        static u64 host_side_alignment();

    protected:
        static u64 alignment_for_usage(BufferUsage usage);

    private:
        u64 _size = 0;
        u64 _offset = 0;
        VkBuffer _buffer = {};
        DeviceMemoryView _memory;
};

// in this case it's ok
//static_assert(is_safe_base<SubBufferBase>::value);

template<BufferUsage Usage = BufferUsage::None, MemoryType Memory = MemoryType::DontCare>
class SubBuffer : public SubBufferBase {

    protected:
        template<typename T>
        static constexpr bool has(T a, T b) {
            return (uenum(a) & uenum(b)) == uenum(b);
        }

        static constexpr bool is_compatible(BufferUsage U, MemoryType M) {
            return has(U, Usage) && is_memory_type_compatible(M, Memory);
        }

        static_assert(!has(BufferUsage::UniformBit, BufferUsage::UniformBit | BufferUsage::IndirectBit));
        static_assert(has(BufferUsage::UniformBit | BufferUsage::IndirectBit, BufferUsage::UniformBit));


        explicit SubBuffer(const BufferBase& buffer) : SubBufferBase(buffer) {
        }

    public:
        static constexpr BufferUsage usage = Usage;
        static constexpr MemoryType memory_type = Memory;

        using sub_buffer_type = SubBuffer<Usage, memory_type>;
        using base_buffer_type = Buffer<Usage, memory_type>;


        static u64 byte_alignment() {
            return alignment_for_usage(Usage);
        }

        static u64 total_byte_size(u64 size) {
            return size;
        }

        SubBuffer() = default;

        template<BufferUsage U, MemoryType M>
        SubBuffer(const SubBuffer<U, M>& buffer) : SubBufferBase(buffer) {
            static_assert(is_compatible(U, M));
        }

        template<BufferUsage U, MemoryType M>
        SubBuffer(const Buffer<U, M>& buffer) : SubBufferBase(buffer, buffer.byte_size(), 0) {
            static_assert(is_compatible(U, M));
        }

        // these are dangerous with typedwrapper as it's never clear what is in byte and what isn't.
        // todo find some way to make this better

        template<BufferUsage U, MemoryType M>
        SubBuffer(const SubBuffer<U, M>& buffer, u64 byte_len, u64 byte_off) : SubBufferBase(buffer, byte_len, byte_off) {
            static_assert(is_compatible(U, M));
            y_debug_assert(buffer.byte_size() >= byte_len);
            y_debug_assert(byte_offset() % byte_alignment() == 0);
            if constexpr(M == MemoryType::CpuVisible) {
                y_debug_assert(byte_offset() % host_side_alignment() == 0);
            }
        }

        template<BufferUsage U, MemoryType M>
        SubBuffer(const Buffer<U, M>& buffer, u64 byte_len, u64 byte_off) : SubBuffer(SubBuffer<U, M>(buffer), byte_len, byte_off) {
        }


};


template<BufferUsage U, MemoryType M>
SubBuffer(const Buffer<U, M>&) -> SubBuffer<U, M>;


}

#endif // YAVE_GRAPHICS_BUFFERS_SUBBUFFER_H

