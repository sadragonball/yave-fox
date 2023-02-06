//
// Created by Xuefujun on 2023/2/6.
//

#include "Slate.h"

#include "editor/utils/ui.h"

namespace editor {
Slate::Slate(std::string_view title, int flags) : _flags(flags) {
  set_title(title);
}
Slate::~Slate() {

}
bool Slate::is_visible() const {
  return _visible;
}
void Slate::set_visible(bool visible) {
  _visible = visible;
}
void Slate::set_parent(Widget *parent) {
  y_debug_assert(!parent != !_parent);
  _parent = parent;
}

void Slate::refresh() {

}
void Slate::refresh_all() {
  Y_TODO(fix refresh)
  refresh();
}
void Slate::on_gui() {
  ImGui::TextUnformatted("Empty widget");
}
bool Slate::before_gui() {
  return true;
}
void Slate::after_gui() {

}
y::math::Vec2ui Slate::content_size() const {
  return (math::Vec2(ImGui::GetWindowContentRegionMax())
      - math::Vec2(ImGui::GetWindowContentRegionMin())).max(math::Vec2(1.0f));;
}
void Slate::draw_imgui_frame() {
  if (!_visible || !before_gui()) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(float(window_size.x()), float(window_size.y())), ImGuiCond_FirstUseEver);

  bool opened = false;

  opened = ImGui::Begin(_title_with_id.data(), &_visible, _flags);

  if (opened) {
    on_gui();
  }

  ImGui::End();
  after_gui();
}

void Slate::set_id(u64 id) {
  _id = id;
  set_title(_title);
}
void Slate::set_title(std::string_view title) {
  _title_with_id = fmt("%##%", title, _id);
  _title = std::string_view(_title_with_id.begin(), title.size());
}
}