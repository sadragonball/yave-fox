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

#include "ImGuiPlatform.h"
#include "Settings.h"

#include <editor/utils/ui.h>

#include <yave/graphics/commands/CmdBufferRecorder.h>
#include <yave/graphics/framebuffer/Framebuffer.h>
#include <yave/window/Monitor.h>

#include <y/io2/File.h>
#include <y/utils/log.h>

#include <external/imgui_test_engine/imgui_te_context.h>
#include <external/imgui_test_engine/imgui_te_exporters.h>
#include <external/imgui_test_engine/imgui_te_internal.h>

#include <deque>




// ---------------------------------------------- TEST HACKS ----------------------------------------------

bool imgui_test_engine_running() {
    const auto* platform = editor::ImGuiPlatform::instance();
    return platform && platform->test_engine() && platform->test_engine()->TestContext;
}



namespace editor {

ImGuiPlatform* imgui_platform() {
    return ImGuiPlatform::instance();
}


// ---------------------------------------------- TEST ENGINE ----------------------------------------------

extern void register_editor_tests(ImGuiTestEngine*);

static ImGuiTestEngine* init_test_engine() {
    ImGuiTestEngine* engine = ImGuiTestEngine_CreateContext();
    ImGuiTestEngineIO& test_io = ImGuiTestEngine_GetIO(engine);
    test_io.ConfigSavedSettings = false;
    test_io.ConfigVerboseLevel = ImGuiTestVerboseLevel_Info;
    test_io.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
    test_io.ConfigBreakOnError = true;

    test_io.ConfigRunSpeed = ImGuiTestRunSpeed_Normal;

    const float speed_scale = 2.0f;
    test_io.MouseSpeed *= speed_scale;
    test_io.TypingSpeed *= speed_scale;
    test_io.ActionDelayShort /= speed_scale;
    test_io.ActionDelayStandard /= speed_scale;

    // Optional: save test output in junit-compatible XML format.
    //test_io.ExportResultsFile = "./results.xml";
    //test_io.ExportResultsFormat = ImGuiTestEngineExportFormat_JUnitXml;

    register_editor_tests(engine);

    // Start test engine
    ImGuiTestEngine_Start(engine, ImGui::GetCurrentContext());
    ImGuiTestEngine_InstallDefaultCrashHandler();

    return engine;
}


// ---------------------------------------------- SETUP HELPERS ----------------------------------------------

static void setup_style() {
    auto& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    const math::Vec4 none = {};

    const float lightness = 1.0f;
    auto rgb = [=](u8 r, u8 g, u8 b, float alpha = 1.0f) {
          return math::Vec4(math::Vec3(r, g, b) / 255.0f * lightness, alpha).min(math::Vec4(1.0f));
    };
    auto grey = [=](u8 g, float alpha = 1.0f) {
          return rgb(g, g, g, alpha);
    };

    for(usize i = 0; i != ImGuiCol_COUNT; ++i) {
        colors[i] = rgb(255, 0, 0);
    }


    colors[ImGuiCol_BorderShadow]           = none;
    colors[ImGuiCol_FrameBgActive]          = none;
    colors[ImGuiCol_Tab]                    = none;
    colors[ImGuiCol_TabUnfocused]           = none;
    colors[ImGuiCol_DockingEmptyBg]         = none;
    colors[ImGuiCol_ModalWindowDimBg]       = none;
    colors[ImGuiCol_TableRowBg]             = none;

    colors[ImGuiCol_Text]                   = grey(180);
    colors[ImGuiCol_TextDisabled]           = grey(90);

    colors[ImGuiCol_CheckMark]              = rgb(0, 112, 224);
    colors[ImGuiCol_DragDropTarget]         = rgb(0, 112, 224);
    colors[ImGuiCol_NavWindowingHighlight]  = rgb(0, 112, 224);

    colors[ImGuiCol_PlotHistogram]          = rgb(0, 112, 224);
    colors[ImGuiCol_PlotHistogramHovered]   = rgb(0, 112, 224);
    colors[ImGuiCol_PlotLines]              = rgb(0, 112, 224);
    colors[ImGuiCol_PlotLinesHovered]       = rgb(0, 112, 224);

    colors[ImGuiCol_NavWindowingHighlight]  = rgb(128, 168, 224);

    colors[ImGuiCol_Border]                 = grey(21, 0.75f);
    colors[ImGuiCol_PopupBg]                = grey(21, 0.9f);
    colors[ImGuiCol_FrameBg]                = grey(15, 0.5f);

    colors[ImGuiCol_NavWindowingDimBg]      = grey(128, 0.75f);
    colors[ImGuiCol_DockingPreview]         = grey(128, 0.25f);
    colors[ImGuiCol_FrameBgHovered]         = grey(128, 0.25f);
    colors[ImGuiCol_TextSelectedBg]         = grey(128, 0.25f);
    colors[ImGuiCol_TableRowBgAlt]          = grey(128, 0.05f);

    colors[ImGuiCol_SliderGrabActive]       = grey(128);
    colors[ImGuiCol_ScrollbarGrabActive]    = grey(128);
    colors[ImGuiCol_ScrollbarGrabHovered]   = grey(128);
    colors[ImGuiCol_ResizeGripActive]       = grey(128);
    colors[ImGuiCol_ResizeGripHovered]      = grey(128);

    colors[ImGuiCol_SliderGrab]             = grey(87);
    colors[ImGuiCol_ScrollbarGrab]          = grey(87);
    colors[ImGuiCol_ResizeGrip]             = grey(87);

    colors[ImGuiCol_Button]                 = grey(60);

    colors[ImGuiCol_ButtonHovered]          = grey(47);
    colors[ImGuiCol_ButtonActive]           = grey(47);
    colors[ImGuiCol_Header]                 = grey(47);
    colors[ImGuiCol_HeaderHovered]          = grey(47);
    colors[ImGuiCol_HeaderActive]           = grey(47);
    colors[ImGuiCol_TableHeaderBg]          = grey(47);
    colors[ImGuiCol_SeparatorActive]        = grey(47);
    colors[ImGuiCol_SeparatorHovered]       = grey(47);
    colors[ImGuiCol_TabHovered]             = grey(47);

    colors[ImGuiCol_WindowBg]               = grey(36);
    colors[ImGuiCol_TabActive]              = grey(36);
    colors[ImGuiCol_TabUnfocusedActive]     = grey(36);

    colors[ImGuiCol_ChildBg]                = grey(28);

    colors[ImGuiCol_Separator]              = grey(26);
    colors[ImGuiCol_TableBorderLight]       = grey(26);
    colors[ImGuiCol_TableBorderStrong]      = grey(26);
    colors[ImGuiCol_ScrollbarBg]            = grey(26);

    colors[ImGuiCol_TitleBg]                = grey(21);
    colors[ImGuiCol_TitleBgActive]          = grey(21);
    colors[ImGuiCol_TitleBgCollapsed]       = grey(21);
    colors[ImGuiCol_MenuBarBg]              = grey(21);


    style.WindowPadding     = ImVec2(4, 4);
    style.FramePadding      = ImVec2(6, 6);
    style.ItemSpacing       = ImVec2(4, 2);

    style.ScrollbarSize     = 12;
    style.ScrollbarRounding = 12;

    style.IndentSpacing     = 12;

    style.WindowBorderSize  = 1;
    style.ChildBorderSize   = 0;
    style.PopupBorderSize   = 0;
    style.FrameBorderSize   = 0;
    style.PopupRounding     = 0;

    style.FrameRounding     = 3;
    style.GrabRounding      = 3;

    style.WindowRounding    = 0;
    style.ChildRounding     = 0;
    style.TabBorderSize     = 0;
    style.TabRounding       = 0;


    if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
}

static void setup_imgui_dockspace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(viewport);
}

static void setup_config_files(ImGuiIO& io) {
    io.IniFilename = "editor.ini";
    io.LogFilename = "editor_logs.txt";
    if(io2::File::open("../editor.ini").is_ok()) {
        io.IniFilename = "../editor.ini";
    }
}

static void setup_backend_flags(ImGuiIO& io, bool multi_viewport) {
    io.BackendPlatformName = "Yave ImGuiPlatform";
    io.BackendRendererName = "Yave";

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = false;

    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_RendererHasViewports;
    if(multi_viewport) {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    }
}

static void discover_monitors(ImGuiPlatformIO& platform) {
    y_profile();

    platform.Monitors.clear();

    auto monitors = Monitor::monitors();
    std::sort(monitors.begin(), monitors.end(), [](const Monitor& a, const Monitor& b) { return b.is_primary < a.is_primary; });
    std::transform(monitors.begin(), monitors.end(), std::back_inserter(platform.Monitors), [](const Monitor& monitor) {
        ImGuiPlatformMonitor imgui_mon;
        imgui_mon.MainPos = monitor.position;
        imgui_mon.MainSize = monitor.size;
        imgui_mon.WorkPos = monitor.work_position;
        imgui_mon.WorkSize = monitor.work_size;
        return imgui_mon;
    });
}


// ---------------------------------------------- EVENT HANDLER ----------------------------------------------

class ImGuiEventHandler : public EventHandler {

    public:
        ImGuiEventHandler(Window* window) : _window(window) {
        }

        void mouse_moved(const math::Vec2i& pos) override {
            const math::Vec2 win_pos = to_window(pos);
            ImGui::GetIO().AddMousePosEvent(win_pos.x(), win_pos.y());
        }

        void mouse_pressed(const math::Vec2i& pos, MouseButton button) override {
            mouse_moved(pos);
            ImGui::GetIO().AddMouseButtonEvent(to_imgui_button(button), true);
        }

        void mouse_released(const math::Vec2i& pos, MouseButton button) override {
            mouse_moved(pos);
            ImGui::GetIO().AddMouseButtonEvent(to_imgui_button(button), false);
        }

        void mouse_wheel(i32 vdelta, i32 hdelta) override {
            ImGui::GetIO().AddMouseWheelEvent(float(hdelta), float(vdelta));
        }

        void char_input(u32 character) override {
            ImGui::GetIO().AddInputCharacter(character);
        }

        void key_pressed(Key key) override {
            ImGui::GetIO().AddKeyEvent(to_imgui_key(key), true);
        }

        void key_released(Key key) override {
            ImGui::GetIO().AddKeyEvent(to_imgui_key(key), false);
        }


    private:
        math::Vec2i to_window(const math::Vec2i& pos) const {
            return ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable
                ? pos + _window->position() : pos;
        }

        Window* _window = nullptr;
};


// ---------------------------------------------- PLATFORM WINDOW ----------------------------------------------

ImGuiPlatform::PlatformWindow::PlatformWindow(ImGuiPlatform* parent, Window::Flags flags) :
        platform(parent),
        window({1280, 768}, "Yave Editor", flags),
        swapchain(&window),
        event_handler(std::make_unique<ImGuiEventHandler>(&window)) {

    window.set_event_handler(event_handler.get());
    window.show();
}

bool ImGuiPlatform::PlatformWindow::render(ImGuiViewport* viewport) {
    y_profile();

    if(!window.update()) {
        return false;
    }

    if(const auto r = swapchain.next_frame()) {
        const FrameToken& token = r.unwrap();
        CmdBufferRecorder recorder = create_disposable_cmd_buffer();

        {
            Framebuffer framebuffer(token.image_view);
            RenderPassRecorder pass = recorder.bind_framebuffer(framebuffer);
            platform->_renderer->render(viewport->DrawData, pass);
        }

        swapchain.present(token, std::move(recorder), command_queue());
    }

    return true;
}


// ---------------------------------------------- PLATFORM ----------------------------------------------

ImGuiPlatform* ImGuiPlatform::_instance = nullptr;

ImGuiPlatform* ImGuiPlatform::instance() {
    y_debug_assert(_instance);
    return _instance;
}

ImGuiPlatform::ImGuiPlatform(bool multi_viewport, bool run_tests) {
    y_profile();

    y_always_assert(_instance == nullptr, "ImGuiPlatform instance already exists.");
    _instance = this;

    ImGui::CreateContext();

    setup_style();

    auto& io = ImGui::GetIO();
    y_always_assert(io.BackendPlatformUserData == nullptr, "ImGui already has a platform backend.");
    io.BackendPlatformUserData = this;
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    setup_config_files(io);
    setup_backend_flags(io, multi_viewport);

    if(multi_viewport) {
        auto& platform = ImGui::GetPlatformIO();

        Y_TODO(do every frame)
        discover_monitors(platform);

        platform.Platform_CreateWindow          = [](ImGuiViewport* vp) {
            ImGuiPlatform* self = get_platform();
            vp->PlatformHandle = self->_windows.emplace_back(std::make_unique<PlatformWindow>(self, Window::NoDecoration)).get();
        };

        platform.Platform_DestroyWindow         = [](ImGuiViewport* vp) { get_platform()->close_window(get_platform_window(vp)); };
        platform.Platform_ShowWindow            = [](ImGuiViewport* vp) { get_window(vp)->show(); };
        platform.Platform_SetWindowPos          = [](ImGuiViewport* vp, ImVec2 pos) { get_window(vp)->set_position(pos); };
        platform.Platform_SetWindowSize         = [](ImGuiViewport* vp, ImVec2 size) { get_window(vp)->set_size(size); };
        platform.Platform_GetWindowPos          = [](ImGuiViewport* vp) { return ImVec2(get_window(vp)->position()); };
        platform.Platform_GetWindowSize         = [](ImGuiViewport* vp) { return ImVec2(get_window(vp)->size()); };

        platform.Platform_SetWindowTitle        = [](ImGuiViewport* vp, const char* title) { get_window(vp)->set_title(title); };

        platform.Platform_SetWindowFocus        = [](ImGuiViewport* vp) { get_window(vp)->focus(); };
        platform.Platform_GetWindowFocus        = [](ImGuiViewport* vp) { return get_window(vp)->has_focus(); };
        platform.Platform_GetWindowMinimized    = [](ImGuiViewport* vp) {return get_window(vp)->is_minimized(); };

        platform.Renderer_RenderWindow          = [](ImGuiViewport* vp, void*) { get_platform_window(vp)->render(vp); };

    }

    _renderer = std::make_unique<ImGuiRenderer>();
    _main_window = std::make_unique<PlatformWindow>(this, Window::Resizable);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    viewport->PlatformHandle = _main_window.get();

    _test_engine = init_test_engine();

    if(run_tests) {
        log_msg("Running ImGui tests", Log::Debug);
        io.IniFilename = "tests.ini";
        ImGuiTestEngine_QueueTests(_test_engine, ImGuiTestGroup_Tests, "tests");
        _running_tests = true;
    }

}

ImGuiPlatform::~ImGuiPlatform() {
    y_always_assert(_instance == this, "ImGuiPlatform instance has already been deleted.");
    _instance = nullptr;
}


const ImGuiRenderer* ImGuiPlatform::renderer() const {
    return _renderer.get();
}

Window* ImGuiPlatform::main_window() {
    return &_main_window->window;
}

const ImGuiTestEngine* ImGuiPlatform::test_engine() const {
    return _test_engine;
}

void ImGuiPlatform::exec(OnGuiFunc func) {
    for(;;) {
        {
            y_profile_zone("frame rate cap");
            const float max_fps = app_settings().editor.max_fps;
            do {
                if(!_main_window->window.update()) {
                    return;
                }
                if(_running_tests && ImGuiTestEngine_GetIO(_test_engine).IsRequestingMaxAppSpeed) {
                    break;
                }
            } while(max_fps > 0.0f && _frame_timer.elapsed().to_secs() < 1.0f / max_fps);
        }

        y_profile_zone("exec once");

        ImGui::GetIO().DeltaTime = std::max(math::epsilon<float>, float(_frame_timer.reset().to_secs()));
        ImGui::GetIO().DisplaySize = _main_window->window.size();

        if(const auto r = _main_window->swapchain.next_frame()) {
            const FrameToken& token = r.unwrap();
            CmdBufferRecorder recorder = create_disposable_cmd_buffer();

            {
                y_profile_zone("imgui");
                ImGui::NewFrame();

                setup_imgui_dockspace();

                if(_demo_window && !_running_tests) {
                    ImGui::ShowDemoWindow(&_demo_window);
                }

                if(func) {
                    func(recorder);
                }

                ImGui::Render();
            }

            {
                y_profile_zone("main window");
                Framebuffer framebuffer(token.image_view);
                RenderPassRecorder pass = recorder.bind_framebuffer(framebuffer);
                _renderer->render(ImGui::GetDrawData(), pass);
            }

            if(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                y_profile_zone("secondary windows");
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            _main_window->swapchain.present(token, std::move(recorder), command_queue());
            ImGuiTestEngine_PostSwap(_test_engine);

            if(_running_tests && ImGuiTestEngine_IsTestQueueEmpty(_test_engine)) {
                int tested = 0;
                int success = 0;
                ImGuiTestEngine_GetResult(_test_engine, tested, success);
                ImGuiTestEngine_PrintResultSummary(_test_engine);

                log_msg(fmt("%/% test passed", success, tested), success == tested ? Log::Debug : Log::Error);
                _running_tests = false;
            }
        }
    }
}


void ImGuiPlatform::close_window(PlatformWindow* window) {
    window->window.close();
    while(window->window.update()) {
        // nothing
    }

    for(auto it = _windows.begin(); it != _windows.end(); ++it) {
        if(it->get() == window) {
            _windows.erase_unordered(it);
            return;
        }
    }

    y_fatal("Window not found.");
}


ImGuiPlatform* ImGuiPlatform::get_platform() {
    y_debug_assert(ImGui::GetIO().BackendPlatformUserData);
    return static_cast<ImGuiPlatform*>(ImGui::GetIO().BackendPlatformUserData);
}

Window* ImGuiPlatform::get_window(ImGuiViewport* vp) {
    return &get_platform_window(vp)->window;
}

ImGuiPlatform::PlatformWindow* ImGuiPlatform::get_platform_window(ImGuiViewport* vp) {
    y_debug_assert(vp->PlatformHandle);
    return static_cast<PlatformWindow*>(vp->PlatformHandle);
}

void ImGuiPlatform::show_demo() {
    _demo_window = true;
}

}



