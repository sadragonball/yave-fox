/*******************************
Copyright (c) 2016-2022 Grégoire Angerand

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
#ifndef EDITOR_UIMANAGER_H
#define EDITOR_UIMANAGER_H

#include "Widget.h"
#include "Slate.h"

#include <y/core/Vector.h>
#include <y/core/HashMap.h>
#include <y/core/FixedArray.h>
#include <y/core/Chrono.h>

#include <typeindex>
#include <memory>

#include <functional>

namespace editor {

class UiManager : NonMovable {

  struct WidgetIdStack {
    core::Vector<u64> released;
    u64 next = 0;
  };

public:
  UiManager();
  ~UiManager();

  void setup_ui();

  void on_gui();

  Widget *add_widget(std::unique_ptr<Widget> widget, bool auto_parent = true);
  Slate *add_slate(std::unique_ptr<Slate> slate);

  void close_all();

private:
  void update_fps_counter();
  void draw_fps_counter();
  void update_shortcuts();
  void draw_menu_bar();
  void set_widget_id(Widget *widget);
  void set_slate_id(Slate *slate);

  core::Vector<std::unique_ptr<Widget>> _widgets;
  core::Vector<std::unique_ptr<Slate>> _slates;
  
  core::FlatHashMap<std::type_index, WidgetIdStack> _ids;

  Widget *_auto_parent = nullptr;

  core::Vector<const EditorAction *> _actions;
  core::Vector<std::pair<const EditorAction *, bool>> _shortcuts;

  core::FixedArray<char> _search_pattern = core::FixedArray<char>(256);

  core::Chrono _timer;
  core::FixedArray<float> _frame_times = core::FixedArray<float>(60);
  float _total_time = 0.0f;
  u64 _frame_number = 0;
};

class FunctionWidget final : public Widget {
public:
  FunctionWidget(std::string_view name, std::function<void()> gui) : Widget(name), _on_gui(std::move(gui)) {
  }

protected:
  void on_gui() override {
    if (_on_gui) {
      _on_gui();
    }
  }

private:
  std::function<void()> _on_gui;
};

}

#endif // EDITOR_UIMANAGER_H

