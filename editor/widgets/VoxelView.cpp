//
// Created by Xuefujun on 2023/2/2.
//

#include "VoxelView.h"

#include "editor/EditorWorld.h"
#include "editor/Settings.h"
#include "editor/EditorApplication.h"
#include "editor/EditorResources.h"
#include "editor/Picker.h"

#include "editor/utils/ui.h"
#include "editor/utils/CameraController.h"

#include "yave/framegraph/FrameGraphFrameResources.h"
#include "yave/framegraph/FrameGraphResourcePool.h"
#include "yave/framegraph/FrameGraphPass.h"
#include "yave/framegraph/FrameGraph.h"

#include "yave/graphics/commands/CmdBufferRecorder.h"
#include "yave/graphics/device/DeviceResources.h"

#include <yave/utils/color.h>

namespace editor {
static std::pair<const char *, math::Vec2ui> standard_resolution() {
  return {"1080p", {1920, 1080}};
}
static bool is_clicked() {
  return ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)
      || ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
}

VoxelView::VoxelView() :
    Slate(ICON_FA_DESKTOP " Voxel View",
          ImGuiWindowFlags_AlwaysAutoResize |
              ImGuiWindowFlags_NoTitleBar |
              ImGuiWindowFlags_NoMove
    ),
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
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg, math::Vec4(0.f));
  ImGui::PushStyleColor(ImGuiCol_Border, math::Vec4(1.f, 0.f, 0.f, 1.f));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, math::Vec2());

  return true;
}

void VoxelView::after_gui() {
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
}

void VoxelView::draw_imgui_frame() {
  if (!get_visible() || !before_gui()) {
    return;
  }
  float view_start_pos_x = ImGui::GetMainViewport()->Pos.x + (ImGui::GetIO().DisplaySize.x * 0.19f);
  float view_start_pos_y = ImGui::GetMainViewport()->Pos.y + 16.0f * 3;
  float height = ImGui::GetIO().DisplaySize.y - 64.f;
  float width = height * 1.2f;

  ImGui::SetNextWindowPos(ImVec2(view_start_pos_x, view_start_pos_y));
  ImGui::SetNextWindowSize
      (ImVec2(width, height),
       ImGuiCond_Always);

  bool opened = false;

  //NULL FOR NO CLOSE
  opened = ImGui::Begin(get_title().data(), NULL, get_flags());

  if (opened) {
    on_gui();
  }

  ImGui::End();
  after_gui();
}

void VoxelView::draw(CmdBufferRecorder &recorder) {
  TextureView *output = nullptr;
  FrameGraph graph(_resource_pool);

  const math::Vec2ui output_size = standard_resolution().second;
  const EditorRenderer renderer = EditorRenderer::create(graph, _scene_view, output_size, _settings);

  {
    const Texture &white = *device_resources()[DeviceResources::WhiteTexture];

    FrameGraphPassBuilder builder = graph.add_pass("ImGui texture pass");

    const auto output_image = builder.declare_image(VK_FORMAT_R8G8B8A8_UNORM, output_size);

    const auto gbuffer = renderer.renderer.gbuffer;
    builder.add_image_input_usage(output_image, ImageUsage::TextureBit);
    builder.add_color_output(output_image);
    builder.add_inline_input(InlineDescriptor(_view), 0);
    builder.add_uniform_input(renderer.final);
    builder.add_uniform_input(gbuffer.depth);
    builder.add_uniform_input(gbuffer.color);
    builder.add_uniform_input(gbuffer.normal);
//    builder.add_uniform_input_with_default(renderer.renderer.ssao.ao, Descriptor(white));
    builder.set_render_func([=, &output](RenderPassRecorder &render_pass, const FrameGraphPass *self) {
      auto out = std::make_unique<TextureView>(self->resources().image<ImageUsage::TextureBit>(output_image));
      output = out.get();
      render_pass.keep_alive(std::move(out));

      const MaterialTemplate *material = resources()[EditorResources::VoxelViewMaterialTemplate];
      render_pass.bind_material_template(material, self->descriptor_sets()[0]);
      render_pass.draw_array(3);
    });
  }

  if (!_disable_render) {
    graph.render(recorder);
  }

  if (output) {
    ImGui::Image(output, content_size());
  }
}

void VoxelView::on_gui() {
  y_profile();

  {
    ImGui::PopStyleVar();
    draw_menu_bar();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, math::Vec2());
  }

  if (ImGui::BeginChild("##view")) {
    update_proj();
    draw(application()->recorder());

    make_drop_target();

    _gizmo.draw();
    _orientation_gizmo.draw();

    update();
  }

  ImGui::EndChild();
}

void VoxelView::draw_settings_menu() {
  if (ImGui::BeginMenu("Tone mapping")) {
    ToneMappingSettings &settings = _settings.renderer_settings.tone_mapping;
    ImGui::MenuItem("Auto exposure", nullptr, &settings.auto_exposure);

    float ev = exposure_to_EV100(settings.exposure);
    if (ImGui::SliderFloat("EV100", &ev, -10.0f, 10.0f)) {
      settings.exposure = EV100_to_exposure(ev);
    }

    ImGui::EndMenu();
  }
}

void VoxelView::draw_menu_bar() {
  if (!(get_flags() & ImGuiWindowFlags_MenuBar)) {
    return;
  }
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Render")) {
      ImGui::MenuItem("Editor entities", nullptr, &_settings.show_editor_entities);

      ImGui::Separator();

      {
        const char *output_names[] = {
            "Alpha Blending", "ISO-Surfaces Extraction"
        };
        for (usize i = 0; i < usize(RenderView::MaxRenderViews); ++i) {
          bool selected = usize(_view) == i;
          ImGui::MenuItem(output_names[i], nullptr, &selected);
          if (selected) {
            _view = RenderView(i);
          }
        }
      }

      ImGui::Separator();
      ImGui::MenuItem("Disable render", nullptr, &_disable_render);

      if (ImGui::BeginMenu("Rendering Settings")) {
        draw_settings_menu();
        ImGui::EndMenu();
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Camera")) {
      if (ImGui::MenuItem("Reset camera")) {
        _scene_view.camera().set_view(Camera().view_matrix());
      }
      ImGui::EndMenu();
    }

// Resolution settings
//    if (ImGui::BeginMenu("Resolution")) {
//
//    }
  }

  draw_gizmo_tool_bar();

  ImGui::Separator();
  ImGui::TextUnformatted(standard_resolution().first);

  ImGui::EndMenuBar();
}

void VoxelView::draw_gizmo_tool_bar() {
  ImGui::Separator();

  auto gizmo_mode = _gizmo.mode();
  auto gizmo_space = _gizmo.space();
  auto snapping = _gizmo.snapping();
  auto rot_snap = _gizmo.rotation_snapping();
  auto center = _gizmo.center_on_object();

  {
    if (ImGui::MenuItem(ICON_FA_ARROWS_ALT, nullptr, false, gizmo_mode != Gizmo::Translate)) {
      gizmo_mode = Gizmo::Translate;
    }

    if (ImGui::MenuItem(ICON_FA_SYNC_ALT, nullptr, false, gizmo_mode != Gizmo::Rotate)) {
      gizmo_mode = Gizmo::Rotate;
    }
  }

  ImGui::Separator();
  {
    if (ImGui::MenuItem(ICON_FA_MOUNTAIN, nullptr, false, gizmo_space != Gizmo::World)) {
      gizmo_space = Gizmo::World;
    }
    if (ImGui::MenuItem(ICON_FA_CUBE, nullptr, false, gizmo_space != Gizmo::Local)) {
      gizmo_space = Gizmo::Local;
    }
  }

  ImGui::Separator();

  {
    if (ImGui::MenuItem(ICON_FA_CROSSHAIRS, nullptr, center)) {
      center = !center;
    }
  }

  if (is_focussed()) {
    const UiSettings &settings = app_settings().ui;
    if (ImGui::IsKeyReleased(to_imgui_key(settings.change_gizmo_mode))) {
      gizmo_mode = Gizmo::Mode(!usize(gizmo_mode));
    }
    if (ImGui::IsKeyReleased(to_imgui_key(settings.change_gizmo_space))) {
      gizmo_space = Gizmo::Space(!usize(gizmo_space));
    }
  }

  _gizmo.set_mode(gizmo_mode);
  _gizmo.set_space(gizmo_space);
  _gizmo.set_snapping(snapping);
  _gizmo.set_rotation_snapping(rot_snap);
  _gizmo.set_center_on_object(center);
}

void VoxelView::update() {
  _gizmo.set_allow_drag(true);

  const bool hovered = ImGui::IsWindowHovered() && is_mouse_inside();

  bool focussed = ImGui::IsWindowFocused();
  if (hovered && is_clicked()) {
    ImGui::SetWindowFocus();
    update_picking();
    focussed = true;
  }

  if (focussed) {
    application()->set_scene_view(&_scene_view);
  }

  if (hovered && !_gizmo.is_dragging() && !_orientation_gizmo.is_dragging() && _camera_controller) {
    auto &camera = _scene_view.camera();
    _camera_controller->process_generic_shortcuts(camera);
    if (focussed) {
      _camera_controller->update_camera(camera, content_size());
    }
  }
}

void VoxelView::update_proj() {
  const CameraSettings &settings = app_settings().camera;
  math::Vec2ui viewport_size = content_size();

  const float fov = math::to_rad(settings.fov);
  const auto proj = math::perspective(fov, float(viewport_size.x()) / float(viewport_size.y()), settings.z_near);
  _scene_view.camera().set_proj(proj);
}

void VoxelView::update_picking() {
  const math::Vec2ui viewport_size = content_size();
  const math::Vec2 offset = ImGui::GetWindowPos();
  const math::Vec2 mouse = ImGui::GetIO().MousePos;
  const math::Vec2 uv = (mouse - offset - math::Vec2(ImGui::GetWindowContentRegionMin())) / math::Vec2(viewport_size);

  if (uv.x() < 0.0f || uv.y() < 0.0f ||
      uv.x() > 1.0f || uv.y() > 1.0f) {
    return;
  }

  const PickingResult picking_data = Picker::pick_sync(_scene_view, uv, viewport_size);
  if (_camera_controller && _camera_controller->viewport_clicked(picking_data)) {
    _gizmo.set_allow_drag(false);
    return;
  }

  if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    if (!_gizmo.is_dragging() && !_orientation_gizmo.is_dragging()) {
      const ecs::EntityId
          picked_id = picking_data.hit() ? current_world().id_from_index(picking_data.entity_index) : ecs::EntityId();
      current_world().toggle_selected(picked_id, !ImGui::GetIO().KeyCtrl);
    }
  }
}

void VoxelView::make_drop_target() {
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(imgui::drag_drop_path_id)) {
      const std::string_view name = reinterpret_cast<const char *>(payload->Data);
      current_world().set_selected(current_world().add_prefab(name));
    }
    ImGui::EndDragDropTarget();
  }
}

}