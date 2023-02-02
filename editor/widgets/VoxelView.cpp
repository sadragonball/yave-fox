//
// Created by Xuefujun on 2023/2/2.
//

#include "VoxelView.h"

#include "editor/EditorWorld.h"
#include "editor/utils/ui.h"
#include "editor/utils/CameraController.h"
#include <editor/EditorApplication.h>

#include "yave/framegraph/FrameGraphResourcePool.h"
#include "yave/framegraph/FrameGraph.h"

namespace editor {
static std::pair<const char *, math::Vec2ui> standard_resolution() {
  return {"1080p", {1920, 1080}};
}
static bool is_clicked() {
  return ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)
      || ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
}

VoxelView::VoxelView() :
    Widget(ICON_FA_DESKTOP "Engine View", ImGuiWindowFlags_MenuBar),
    _resource_pool(std::make_shared<FrameGraphResourcePool>()),
    _scene_view(&current_world()),
    _camera_controller(std::make_unique<HoudiniCameraController>()),
    _gizmo(&_scene_view),
    _orientation_gizmo(&_scene_view) {
}

VoxelView::~VoxelView() noexcept {
  application()->unset_scene_view(&_scene_view);
}

bool VoxelView::is_mouse_inside() const {
  const math::Vec2 mouse_pos = math::Vec2(ImGui::GetIO().MousePos) - math::Vec2(ImGui::GetWindowPos());
  const auto less = [](const math::Vec2 &a, const math::Vec2 &b) { return a.x() < b.x() && a.y() < b.y(); };
  return less(mouse_pos, ImGui::GetWindowContentRegionMax()) && less(ImGui::GetWindowContentRegionMin(), mouse_pos);
}

bool VoxelView::is_focussed() const {
  return ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
}

bool VoxelView::before_gui() {
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg, math::Vec4(0.0f));
  ImGui::PushStyleColor(ImGuiCol_Border, math::Vec4(0.0f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, math::Vec2());

  return true;
}

void VoxelView::after_gui() {
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
}

void VoxelView::draw(CmdBufferRecorder &recorder) {
  TextureView *output = nullptr;
  FrameGraph graph(_resource_pool);

  const math::Vec2ui output_size = standard_resolution().second;
  const EditorRenderer renderer = EditorRenderer::create(graph, _scene_view, output_size, _settings);
}
}