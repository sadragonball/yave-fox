/*******************************
Copyright (C) 2013-2016 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/
#ifndef YAVE_FRAMEBUFFER_H
#define YAVE_FRAMEBUFFER_H

#include "yave.h"

#include "RenderPass.h"

#include <yave/image/ImageView.h>

namespace yave {

class Framebuffer : NonCopyable, public DeviceLinked {

	Y_TODO(Framebuffer does not need DeviceLinked)

	public:
		Framebuffer() = default;
		Framebuffer(DevicePtr dptr, RenderPass& render_pass, DepthAttachmentView depth, ColorAttachmentView color);

		Framebuffer(Framebuffer&& other);
		Framebuffer& operator=(Framebuffer&& other);

		~Framebuffer();

		const math::Vec2ui &size() const;
		const RenderPass &get_render_pass() const;


		vk::Framebuffer get_vk_framebuffer() const;

	private:
		void swap(Framebuffer& other);

		math::Vec2ui _size;
		NotOwned<RenderPass*> _render_pass;
		vk::Framebuffer _framebuffer;
};

}

#endif // YAVE_FRAMEBUFFER_H
