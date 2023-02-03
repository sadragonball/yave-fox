//
// Created by Xuefujun on 2023/2/2.
//

#ifndef YAVE_EDITOR_WIDGETS_VOXELVIEW_H
#define YAVE_EDITOR_WIDGETS_VOXELVIEW_H
#include "Gizmo.h"
#include "OrientationGizmo.h"

#include "editor/Widget.h"
#include "editor/renderer/EditorRenderer.h"

namespace editor {
class VoxelView final : public Widget {
  editor_widget_open(VoxelView, "View")
public:
  enum class RenderView : u32 {
    ISOSurface,
    MaxRenderViews
  };

  VoxelView();
  ~VoxelView();

protected:
  void on_gui() override;

  bool before_gui() override;
  void after_gui() override;

private:
  void draw(CmdBufferRecorder &recorder);
  void draw_menu_bar();
  void draw_settings_menu();
  void draw_gizmo_tool_bar();

  void update_proj();
  void update();
  void update_picking();

  bool is_mouse_inside() const;
  bool is_focussed() const;

  void make_drop_target();

  RenderView _view = RenderView::AlphaBlending;

  std::shared_ptr<FrameGraphResourcePool> _resource_pool;

  EditorRendererSettings _settings;

  SceneView _scene_view;
  std::unique_ptr<CameraController> _camera_controller;

  Gizmo _gizmo;
  OrientationGizmo _orientation_gizmo;

  bool _disable_render = false;
};
}

#endif //YAVE_EDITOR_WIDGETS_VOXELVIEW_H
