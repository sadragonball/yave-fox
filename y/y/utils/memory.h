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
#ifndef Y_UTILS_MEMORY_H
#define Y_UTILS_MEMORY_H

#include <y/utils.h>

namespace y {

constexpr usize max_alignment = std::alignment_of<std::max_align_t>::value;

template<typename U>
constexpr U align_up_to(U value, U alignment) {
    y_debug_assert(alignment);
    if(const U diff = value % alignment) {
        y_debug_assert(diff <= value + alignment);
        return value + alignment - diff;
    }
    return value;
}

template<typename U>
constexpr U align_down_to(U value, U alignment) {
    y_debug_assert(alignment);
    const U diff = value % alignment;
    return value - diff;
}

template<typename U>
constexpr U align_up_to_max(U size) {
    return align_up_to(size, U(max_alignment));
}

// https://en.wikipedia.org/wiki/Hamming_weight
static inline u32 popcnt_32(u32 x) {
    x -= (x >> 1) & 0x55555555;
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f;
    return (x * 0x01010101) >> 24;
}

static inline u64 popcnt_64(u64 x) {
    const u64 m1  = 0x5555555555555555;
    const u64 m2  = 0x3333333333333333;
    const u64 m4  = 0x0f0f0f0f0f0f0f0f;
    const u64 h01 = 0x0101010101010101;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    return (x * h01) >> 56;
}


}

#endif // Y_UTILS_MEMORY_H

