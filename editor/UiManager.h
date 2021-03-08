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
#ifndef EDITOR_UIMANAGER_H
#define EDITOR_UIMANAGER_H

#include "Widget.h"

#include <y/core/Vector.h>
#include <y/core/HashMap.h>

#include <typeindex>
#include <memory>

namespace editor {

class UiDebugWidget : public Widget {
    public:
        UiDebugWidget();
        void draw_gui() override;
};

class UiManager : NonMovable {

    struct WidgetIdStack {
        core::Vector<u64> released;
        u64 next = 0;
    };

    public:
        UiManager();
        ~UiManager();

        void draw_gui();

        Widget* add_widget(std::unique_ptr<Widget> widget, bool auto_parent = true);

        // y_reflect(_widgets);

    private:
        void set_widget_id(Widget* widget);

        core::Vector<std::unique_ptr<Widget>> _widgets;
        core::ExternalHashMap<std::type_index, WidgetIdStack> _ids;

        Widget* _auto_parent = nullptr;
};


}

#endif // EDITOR_UIMANAGER_H

