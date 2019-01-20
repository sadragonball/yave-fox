/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

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

#include "ThumbmailCache.h"

#include <editor/context/EditorContext.h>

#include <yave/scene/SceneView.h>
#include <yave/renderer/ToneMappingPass.h>

#include <thread>

namespace editor {

ThumbmailCache::Thumbmail::Thumbmail(DevicePtr dptr, usize size, AssetId asset) :
		image(dptr, vk::Format::eR8G8B8A8Unorm, math::Vec2ui(size)),
		view(image),
		id(asset) {
}

ThumbmailCache::SceneData::SceneData(DevicePtr dptr, const AssetPtr<StaticMesh>& mesh)
	: view(scene) {

	scene.static_meshes() << std::make_unique<StaticMeshInstance>(mesh, dptr->default_resources()[DefaultResources::BasicMaterial]);
	view.camera().set_view(math::look_at(math::Vec3(mesh->radius() * 1.5f), math::Vec3(0.0f), math::Vec3(0.0f, 0.0f, 1.0f)));
}


static auto load_envmap() {
	math::Vec4 data(1.0f, 1.0f, 1.0f, 1.0f);
	return ImageData(math::Vec2ui(1), reinterpret_cast<const u8*>(data.data()), vk::Format::eR32G32B32A32Sfloat);
}

ThumbmailCache::ThumbmailCache(ContextPtr ctx, usize size) :
	ContextLinked(ctx),
	_size(size),
	_ibl_data(std::make_shared<IBLData>(device(), load_envmap())) {
}

math::Vec2ui ThumbmailCache::thumbmail_size() const {
	return math::Vec2ui(_size);
}

TextureView* ThumbmailCache::get_thumbmail(AssetId asset) {
	if(!asset.is_valid()) {
		return nullptr;
	}

	process_requests();
	if(auto it = _thumbmails.find(asset); it != _thumbmails.end()) {
		if(it->second) {
			return &it->second->view;
		} else {
			return nullptr;
		}
	}
	request_thumbmail(asset);
	return nullptr;
}

void ThumbmailCache::process_requests() {
	std::unique_ptr<CmdBufferRecorder> recorder;
	for(usize i = 0; i != _requests.size(); ++i) {
		if(_requests[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
			if(!recorder) {
				recorder = std::make_unique<CmdBufferRecorder>(device()->create_disposable_cmd_buffer());
			}

			ThumbmailFunc func = _requests[i].get();
			auto thumb = func(*recorder);
			_thumbmails[thumb->id] = std::move(thumb);

			_requests.erase_unordered(_requests.begin() + i);
			--i;
		}
	}
	if(recorder) {
		device()->graphic_queue().submit<SyncSubmit>(std::move(*recorder));
	}
}


void ThumbmailCache::request_thumbmail(AssetId asset) {
	_thumbmails[asset] = nullptr;
	_requests << std::async(std::launch::async, [=]() -> ThumbmailFunc {
			try {
				auto mesh = context()->loader().static_mesh().load(asset);
				return [=](CmdBufferRecorder& rec) { return render_thumbmail(rec, mesh); };
			} catch(...) {
			}

			try {
				auto tex = context()->loader().texture().load(asset);
				return [=](CmdBufferRecorder& rec) { return render_thumbmail(rec, tex); };
			} catch(...) {
			}

			return [](CmdBufferRecorder&) { return nullptr; };
		});
}

std::unique_ptr<ThumbmailCache::Thumbmail> ThumbmailCache::render_thumbmail(CmdBufferRecorder& recorder, const AssetPtr<Texture>& tex) {
	auto thumbmail = std::make_unique<Thumbmail>(device(), _size, tex.id());

	{
		DescriptorSet set(device(), {Binding(*tex), Binding(StorageView(thumbmail->image))});
		recorder.dispatch_size(device()->default_resources()[DefaultResources::CopyProgram],  math::Vec2ui(_size), {set});
		recorder.keep_alive(std::move(set));
	}

	return thumbmail;
}

std::unique_ptr<ThumbmailCache::Thumbmail> ThumbmailCache::render_thumbmail(CmdBufferRecorder& recorder, const AssetPtr<StaticMesh>& mesh) {
	auto thumbmail = std::make_unique<Thumbmail>(device(), _size, mesh.id());
	SceneData scene(device(), mesh);

	{
		auto region = recorder.region("ThumbmailCache::render");

		FrameGraph graph(context()->resource_pool());
		auto gbuffer = render_gbuffer(graph, &scene.view, thumbmail->image.size());
		auto lighting = render_lighting(graph, gbuffer, _ibl_data);
		auto tone_mapping = render_tone_mapping(graph, lighting);

		FrameGraphImageId output_image = tone_mapping.tone_mapped;
		{
			FrameGraphPassBuilder builder = graph.add_pass("Thumbmail copy pass");
			builder.add_uniform_input(output_image);
			builder.add_uniform_input(gbuffer.depth);
			builder.add_uniform_input(StorageView(thumbmail->image));
			builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {
					recorder.dispatch_size(device()->default_resources()[DefaultResources::DepthAlphaProgram], math::Vec2ui(_size), {self->descriptor_sets()[0]});
				});
		}

		std::move(graph).render(recorder);
	}

	return thumbmail;
}


}
