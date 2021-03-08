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

#ifndef EDITOR_EDITOR_H
#define EDITOR_EDITOR_H

#include <yave/yave.h>
#include <editor/utils/forward.h>

#include <memory>

namespace editor {

using namespace yave;

EditorApplication* application();
DevicePtr app_device();

Settings& app_settings();
Selection& selection();


AssetStore& asset_store();
AssetLoader& asset_loader();
EditorWorld& world();
const SceneView& scene_view();


const EditorResources& resources();
UiManager& ui();
ImGuiPlatform* imgui_platform();


Widget* add_widget(std::unique_ptr<Widget> widget, bool auto_parent = true);

template<typename T, typename... Args>
T* add_child_widget(Args&&... args) {
    return dynamic_cast<T*>(add_widget(std::make_unique<T>(y_fwd(args)...), true));
}

template<typename T, typename... Args>
T* add_detached_widget(Args&&... args) {
    return dynamic_cast<T*>(add_widget(std::make_unique<T>(y_fwd(args)...), false));
}

}


#endif // EDITOR_EDITOR_H
