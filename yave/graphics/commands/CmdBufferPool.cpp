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

#include "CmdBufferPool.h"
#include "CmdBuffer.h"

#include <yave/graphics/utils.h>
#include <yave/graphics/device/Queue.h>

#include <y/core/Chrono.h>
#include <y/concurrent/concurrent.h>

namespace yave {

static VkCommandPool create_pool(DevicePtr dptr) {
    VkCommandPoolCreateInfo create_info = vk_struct();
    {
        create_info.queueFamilyIndex = graphic_queue(dptr).family_index();
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    VkCommandPool pool = {};
    vk_check(vkCreateCommandPool(vk_device(dptr), &create_info, vk_allocation_callbacks(dptr), &pool));
    return pool;
}



CmdBufferPool::CmdBufferPool() : _thread_id(concurrent::thread_id()) {
}

CmdBufferPool::CmdBufferPool(DevicePtr dptr) :
        DeviceLinked(dptr),
        _pool(create_pool(dptr)),
        _thread_id(concurrent::thread_id()) {
}

CmdBufferPool::~CmdBufferPool() {
    if(device()) {
        if(_fences.size() != _cmd_buffers.size()) {
            y_fatal("CmdBuffers are still in use (% fences, % buffers).", _fences.size(), _cmd_buffers.size());
        }
        join_all();
        _cmd_buffers.clear();
        destroy(_pool);
    }
}

VkCommandPool CmdBufferPool::vk_pool() const {
    return _pool;
}

void CmdBufferPool::join_all() {
    if(_fences.is_empty()) {
        return;
    }

    if(vkWaitForFences(vk_device(device()), _fences.size(), _fences.data(), true, u64(-1)) != VK_SUCCESS) {
        y_fatal("Unable to join fences.");
    }
}

void CmdBufferPool::release(std::unique_ptr<CmdBufferData> data) {
    y_profile();

    if(data->pool() != this) {
        y_fatal("CmdBufferData was not returned to its original pool.");
    }
    data->release_resources();

    const auto lock = y_profile_unique_lock(_lock);
    _cmd_buffers.push_back(std::move(data));
}

std::unique_ptr<CmdBufferData> CmdBufferPool::alloc() {
    y_profile();

    y_debug_assert(concurrent::thread_id() == _thread_id);
    {
        auto lock = y_profile_unique_lock(_lock);
        if(!_cmd_buffers.is_empty()) {
            auto data = _cmd_buffers.pop();
            lock.unlock();

            data->reset();
            return data;
        }
    }

    return create_data();
}

std::unique_ptr<CmdBufferData> CmdBufferPool::create_data() {
    const VkFenceCreateInfo fence_create_info = vk_struct();
    VkCommandBufferAllocateInfo allocate_info = vk_struct();
    {
        allocate_info.commandBufferCount = 1;
        allocate_info.commandPool = _pool;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    VkCommandBuffer buffer = {};
    VkFence fence = {};
    vk_check(vkAllocateCommandBuffers(vk_device(device()), &allocate_info, &buffer));
    vk_check(vkCreateFence(vk_device(device()), &fence_create_info, vk_allocation_callbacks(device()), &fence));

    _fences << fence;
    return std::make_unique<CmdBufferData>(buffer, fence, this);
}


CmdBuffer CmdBufferPool::create_buffer() {
    return CmdBuffer(alloc());
}

}

