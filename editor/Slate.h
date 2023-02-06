//
// Created by Xuefujun on 2023/2/6.
//

#ifndef YAVE_EDITOR_SLATE_H
#define YAVE_EDITOR_SLATE_H

#include "editor.h"

#include "y/core/String.h"

namespace editor {
class Slate : NonMovable {
public:
  Slate(std::string_view title, int flags = 0);
  virtual ~Slate();

  bool is_visible() const;

  void set_visible(bool visible);

  void set_parent(Widget *parent);

  virtual void refresh();
  virtual void refresh_all();

protected:
  friend class UiManager;

  virtual void draw_imgui_frame();
  virtual void on_gui();
  virtual bool before_gui();
  virtual void after_gui();

  [[nodiscard]] math::Vec2ui content_size() const;
  [[nodiscard]] math::Vec2ui get_window_size() const {
    return window_size;
  }

  [[nodiscard]] bool *get_visible() {
    return &_visible;
  }

  [[nodiscard]] std::string_view get_title() const {
    return _title;
  }

  int get_flags() {
    return _flags;
  }

  void set_flags(int flags) {
    _flags |= flags;
  };

private:
  core::String _title_with_id;
  std::string_view _title;
  bool _visible = true;
  math::Vec2ui window_size = {520, 600};

  void set_id(u64 id);
  void set_title(std::string_view title);

  Widget *_parent = nullptr;
  int _flags = 0;
  u64 _id = 0;
};

}
#endif //YAVE_EDITOR_SLATE_H
