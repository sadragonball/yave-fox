/*******************************
Copyright (c) 2016-2021 Grégoire Angerand

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

#include "CmdBufferData.h"

#include <yave/graphics/commands/CmdBufferPool.h>
#include <yave/graphics/device/LifetimeManager.h>
#include <yave/graphics/utils.h>

#include <y/utils/log.h>

namespace yave {

CmdBufferData::CmdBufferData(VkCommandBuffer buf, VkFence fen, CmdBufferPool* p) :
        _cmd_buffer(buf), _fence(fen), _pool(p), _resource_fence(lifetime_manager(device()).create_fence()) {
}

CmdBufferData::~CmdBufferData() {
    if(_pool) {
        y_debug_assert(vkGetFenceStatus(vk_device(device()), _fence) == VK_SUCCESS);
    }
}

DevicePtr CmdBufferData::device() const {
    return _pool ? _pool->device() : nullptr;
}

bool CmdBufferData::is_null() const {
    return !device();
}

bool CmdBufferData::is_signaled() const {
    return _signaled;
}

CmdBufferPool* CmdBufferData::pool() const {
    return _pool;
}

VkCommandBuffer CmdBufferData::vk_cmd_buffer() const {
    return _cmd_buffer;
}

VkFence CmdBufferData::vk_fence() const {
    return _fence;
}

ResourceFence CmdBufferData::resource_fence() const {
    return _resource_fence;
}

void CmdBufferData::wait() {
    if(!is_signaled()) {
        vk_check(vkWaitForFences(vk_device(device()), 1, &_fence, true, u64(-1)));
        set_signaled();
    }
}

bool CmdBufferData::poll_and_signal() {
    if(is_signaled()) {
        return true;
    }
    if(poll()) {
        set_signaled();
        return true;
    }
    return false;
}

bool CmdBufferData::poll() const {
    return vkGetFenceStatus(vk_device(device()), _fence) == VK_SUCCESS;
}

void CmdBufferData::begin() {
    y_profile();

    y_debug_assert(is_signaled());
    y_debug_assert(!_polling);

    recycle_resources(); // just in case

    vk_check(vkResetFences(vk_device(device()), 1, &_fence));
    vk_check(vkResetCommandBuffer(_cmd_buffer, 0));
    _signaled = false;

    _resource_fence = lifetime_manager(device()).create_fence();
}

void CmdBufferData::recycle_resources() {
    y_profile();

    y_debug_assert(is_signaled());

    _keep_alive.clear();
}

void CmdBufferData::set_signaled() {
    const bool previous = _signaled.exchange(true, std::memory_order_acquire);
    if(!previous) {
        recycle_resources();
        lifetime_manager(device()).release_fence(_resource_fence);
    }
}

}

