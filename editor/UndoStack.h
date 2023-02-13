/*******************************
Copyright (c) 2016-2023 Grégoire Angerand

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
#ifndef EDITOR_UNDOSTACK_H
#define EDITOR_UNDOSTACK_H

#include <editor/editor.h>

#include <yave/ecs/EntityPrefab.h>

#include <y/io2/Buffer.h>

namespace editor {

class UndoStackWidget;

class UndoStack : NonMovable {

    struct StackItem {
        ecs::EntityId id;
        ecs::EntityPrefab prefab;
    };

    public:
        UndoStack();

        void clear();

        void set_editing_entity(ecs::EntityId id);

        void done_editing();
        void make_dirty();

        bool is_entity_dirty() const;

        void push_before_dirty(ecs::EntityId id);

        bool can_undo() const;
        void undo();

        bool can_redo() const;
        void redo();


        UndoStack& operator<<(bool dirty) {
            if(dirty) {
                make_dirty();
            }
            return *this;
        }

    private:
        friend class UndoStackWidget;
        void restore_entity();
};

}

#endif // EDITOR_UNDOSTACK_H

