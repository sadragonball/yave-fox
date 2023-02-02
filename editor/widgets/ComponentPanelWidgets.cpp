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

#include "ComponentPanelWidgets.h"
#include "AssetSelector.h"

#include <editor/UndoStack.h>
#include <editor/EditorWorld.h>
#include <editor/components/EditorComponent.h>
#include <editor/utils/ui.h>
#include <editor/widgets/FileBrowser.h>

#include <yave/components/PointLightComponent.h>
#include <yave/components/SpotLightComponent.h>
#include <yave/components/DirectionalLightComponent.h>
#include <yave/components/SkyLightComponent.h>
#include <yave/components/StaticMeshComponent.h>
#include <yave/components/TransformableComponent.h>
#include <yave/components/AtmosphereComponent.h>

#include <yave/utils/color.h>
#include <yave/assets/AssetLoader.h>
#include <yave/graphics/images/ImageData.h>
#include <yave/material/Material.h>
#include <yave/meshes/StaticMesh.h>
#include <yave/meshes/MeshData.h>

#include <y/io2/File.h>

#include <y/utils/log.h>
#include <y/utils/format.h>



#include <cinttypes>

namespace editor {

ComponentPanelWidgetBase::Link* ComponentPanelWidgetBase::_first_link = nullptr;

void ComponentPanelWidgetBase::register_link(Link* link) {
    link->next = _first_link;
    _first_link = link;
}


core::String ComponentPanelWidgetBase::component_name() const {
    return runtime_info().clean_component_name();
}


const ImGuiTableFlags table_flags =
        ImGuiTableFlags_BordersInnerV |
        // ImGuiTableFlags_BordersInnerH |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_RowBg;

template<typename CRTP, typename T>
struct LightComponentWidget : public ComponentPanelWidget<CRTP, T> {
    static void basic_properties(T* light, bool directional) {
        const int color_flags = ImGuiColorEditFlags_NoSidePreview |
                          // ImGuiColorEditFlags_NoSmallPreview |
                          ImGuiColorEditFlags_NoAlpha |
                          ImGuiColorEditFlags_Float |
                          ImGuiColorEditFlags_InputRGB;

        const math::Vec4 color(light->color(), 1.0f);

        {
            ImGui::TextUnformatted("Light color");
            ImGui::TableNextColumn();

            if(ImGui::ColorButton("Color", color, color_flags)) {
                ImGui::OpenPopup("##color");
            }

            if(ImGui::BeginPopup("##color")) {
                ImGui::ColorPicker3("##lightpicker", light->color().begin(), color_flags);

                float kelvin = std::clamp(rgb_to_k(light->color()), 1000.0f, 12000.0f);
                if(ImGui::SliderFloat("##temperature", &kelvin, 1000.0f, 12000.0f, "%.0f°K")) {
                    light->color() = k_to_rbg(kelvin);
                }

                ImGui::EndPopup();
            }
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Intensity");
            ImGui::TableNextColumn();
            if(directional) {
                ImGui::DragFloat("##intensity", &light->intensity(), 1.0f, 0.0f, std::numeric_limits<float>::max(), "%.2f lm/m²");
            } else {
                const float factor = 4.0f * math::pi<float>;
                float intensity = light->intensity() * factor;
                ImGui::DragFloat("##intensity", &intensity, 1.0f, 0.0f, std::numeric_limits<float>::max(), "%.2f lm");
                light->intensity() = intensity / factor;
            }
        }
    }

    static void shadow_properties(T* light) {

        {
            ImGui::TextUnformatted("Cast shadows");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##shadows", &light->cast_shadow());
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Shadow LoD");
            ImGui::TableNextColumn();

            int lod = light->shadow_lod();
            if(ImGui::DragInt("##lod", &lod, 1.0f / 8.0f, 0, 8)) {
                light->shadow_lod() = lod;
            }
        }
    }
};

struct PointLightComponentWidget : public LightComponentWidget<PointLightComponentWidget, PointLightComponent> {
    void on_gui(ecs::EntityId, PointLightComponent* light) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        basic_properties(light, false);

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Range");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##range", &light->range(), 1.0f, 0.0f, std::numeric_limits<float>::max(), "%.2f m");
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Falloff");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##falloff", &light->falloff(), 0.1f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
        }
    }
};

struct DirectionalLightComponentWidget : public LightComponentWidget<DirectionalLightComponentWidget, DirectionalLightComponent> {
    void on_gui(ecs::EntityId, DirectionalLightComponent* light) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        basic_properties(light, true);

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Direction");
            ImGui::TableNextColumn();

            const math::Vec3 dir = light->direction().normalized();
            float elevation = -math::to_deg(std::asin(dir.z()));
            const math::Vec2 dir_2d = dir.to<2>().normalized();
            float azimuth = math::to_deg(std::copysign(std::acos(dir_2d.x()), std::asin(dir_2d.y())));

            bool changed = false;

            changed |= ImGui::DragFloat("Azimuth", &azimuth, 1.0, -180.0f, 180.0f, "%.2f°");
            changed |= ImGui::DragFloat("Elevation", &elevation, 1.0, -90.0f, 90.0f, "%.2f°");

            if(changed) {
                elevation = -math::to_rad(elevation);
                azimuth = math::to_rad(azimuth);

                math::Vec3 new_dir;
                new_dir.z() = std::sin(elevation);
                new_dir.to<2>() = math::Vec2(std::cos(azimuth), std::sin(azimuth)) * std::cos(elevation);

                light->direction() = new_dir;
            }
        }

        imgui::table_begin_next_row();

        shadow_properties(light);

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("First cascade distance");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##firstdist", &light->first_cascade_distance(), 1.0f, 10.0f, std::numeric_limits<float>::max(), "%.2f m");
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Last cascade distance");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##lastdist", &light->last_cascade_distance(), 1.0f, 10.0f, std::numeric_limits<float>::max(), "%.2f m");
        }

    }
};

struct SpotLightComponentWidget : public LightComponentWidget<SpotLightComponentWidget, SpotLightComponent> {
    void on_gui(ecs::EntityId, SpotLightComponent* light) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        basic_properties(light, false);

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Range");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##range", &light->range(), 1.0f, 0.0f, std::numeric_limits<float>::max(), "%.2f m");
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Falloff");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##falloff", &light->falloff(), 0.1f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Outer angle");
            ImGui::TableNextColumn();
            float angle = math::to_deg(light->half_angle() * 2.0f);
            if(ImGui::DragFloat("##outer_angle", &angle, 0.1f, 0.0f, 360.0f, "%.2f°")) {
                light->half_angle() = math::to_rad(angle * 0.5f);
                // light->half_inner_angle() = std::min(light->half_inner_angle(), light->half_angle());
            }
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Inner angle");
            ImGui::TableNextColumn();
            float angle = math::to_deg(light->half_inner_angle() * 2.0f);
            if(ImGui::DragFloat("##inner_angle", &angle, 0.1f, 0.0f, 360.0f, "%.2f°", ImGuiSliderFlags_AlwaysClamp)) {
                light->half_inner_angle() = math::to_rad(angle * 0.5f);
                // light->half_angle() = std::max(light->half_inner_angle(), light->half_angle());
            }
        }

        imgui::table_begin_next_row();

        shadow_properties(light);
    }
};

struct SkyLightComponentWidget : public ComponentPanelWidget<SkyLightComponentWidget, SkyLightComponent> {
    void on_gui(ecs::EntityId id, SkyLightComponent* sky) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        {
            ImGui::TextUnformatted("Envmap");
            ImGui::TableNextColumn();

            bool clear = false;
            if(imgui::asset_selector(sky->probe().id(), AssetType::Image, "Envmap", &clear)) {
                add_child_widget<AssetSelector>(AssetType::Image)->set_selected_callback(
                    [=](AssetId asset) {
                        if(const auto probe = asset_loader().load_res<IBLProbe>(asset)) {
                            if(SkyLightComponent* sky = current_world().component_mut<SkyLightComponent>(id)) {
                                sky->probe() = probe.unwrap();
                            }
                        }
                        return true;
                    });
            } else if(clear) {
                sky->probe() = AssetPtr<IBLProbe>();
            }
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Intensity");
            ImGui::TableNextColumn();
            ImGui::DragFloat("##intensity", &sky->intensity(), 1.0f, 0.0f, std::numeric_limits<float>::max(), "%.2f");
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Display sky");
            ImGui::TableNextColumn();
            ImGui::Checkbox("##displaysky", &sky->display_sky());
        }
    }
};

struct StaticMeshComponentWidget : public ComponentPanelWidget<StaticMeshComponentWidget, StaticMeshComponent> {
    void on_gui(ecs::EntityId id, StaticMeshComponent* static_mesh) {


        if(ImGui::BeginTable("#mesh", 2, table_flags)) {
            imgui::table_begin_next_row();

            ImGui::TextUnformatted("Mesh");
            ImGui::TableNextColumn();

            bool clear = false;
            if(imgui::asset_selector(static_mesh->mesh().id(), AssetType::Mesh, "Mesh", &clear)) {
                add_child_widget<AssetSelector>(AssetType::Mesh)->set_selected_callback(
                    [=](AssetId asset) {
                        if(const auto mesh = asset_loader().load_res<StaticMesh>(asset)) {
                            if(StaticMeshComponent* static_mesh = current_world().component_mut<StaticMeshComponent>(id)) {
                                static_mesh->mesh() = mesh.unwrap();
                            }
                        }
                        return true;
                    });
            } else if(clear) {
                static_mesh->mesh() = AssetPtr<StaticMesh>();
            }

            ImGui::EndTable();
        }

        if(ImGui::CollapsingHeader("Materials")) {
            if(ImGui::BeginTable("#materials", 2, table_flags)) {
                const auto materials = static_mesh->materials();
                for(usize i = 0; i != materials.size(); ++i) {
                    imgui::table_begin_next_row();

                    const char* name = fmt_c_str("Material #%", i);

                    ImGui::TextUnformatted(name);
                    ImGui::TableNextColumn();

                    bool clear = false;
                    if(imgui::asset_selector(materials[i].id(), AssetType::Material, name, &clear)) {
                        add_child_widget<AssetSelector>(AssetType::Material)->set_selected_callback(
                            [=](AssetId asset) {
                                if(const auto mat = asset_loader().load_res<Material>(asset)) {
                                    if(StaticMeshComponent* static_mesh = current_world().component_mut<StaticMeshComponent>(id)) {
                                        static_mesh->materials()[i] = mat.unwrap();
                                    }
                                }
                                return true;
                            });
                    } else if(clear) {
                        materials[i] = AssetPtr<Material>();
                    }
                }

                ImGui::EndTable();
            }
        }
    }
};

struct AtmosphereComponentWidget : public ComponentPanelWidget<AtmosphereComponentWidget, AtmosphereComponent> {
    void on_gui(ecs::EntityId , AtmosphereComponent* component) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        ImGui::TextUnformatted("Planet radius");
        ImGui::TableNextColumn();
        ImGui::InputFloat("##radius", &component->planet_radius, 0.0f, 0.0f, "%.6f km");

        imgui::table_begin_next_row();

        ImGui::TextUnformatted("Atmosphere height");
        ImGui::TableNextColumn();
        ImGui::InputFloat("##height", &component->planet_radius, 0.0f, 0.0f, "%.6f km");

        imgui::table_begin_next_row();

        ImGui::TextUnformatted("Density falloff");
        ImGui::TableNextColumn();
        ImGui::InputFloat("##density", &component->planet_radius, 0.0f, 0.0f, "%.6f km");

        imgui::table_begin_next_row();

        ImGui::TextUnformatted("Scaterring strength");
        ImGui::TableNextColumn();
        ImGui::InputFloat("##strength", &component->planet_radius, 0.0f, 0.0f, "%.6f km");
    }
};

struct TransformableComponentWidget : public ComponentPanelWidget<TransformableComponentWidget, TransformableComponent> {
    void on_gui(ecs::EntityId id, TransformableComponent* component) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        auto [pos, rot, scale] = component->transform().decompose();

        // position
        {
            ImGui::TextUnformatted("Position");
            ImGui::TableNextColumn();

            imgui::position_input("##position", pos);
        }

        imgui::table_begin_next_row();

        // rotation
        {
            ImGui::TextUnformatted("Rotation");
            ImGui::TableNextColumn();

            auto is_same_angle = [&](math::Vec3 a, math::Vec3 b) {
                const auto qa = math::Quaternion<>::from_euler(a);
                const auto qb = math::Quaternion<>::from_euler(b);
                for(usize i = 0; i != 3; ++i) {
                    math::Vec3 v;
                    v[i] = 1.0f;
                    if((qa(v) - qb(v)).length2() > 0.001f) {
                        return false;
                    }
                }
                return true;
            };

            auto& euler = current_world().component_mut<EditorComponent>(id)->euler();

            {
                math::Vec3 actual_euler = rot.to_euler();
                if(!is_same_angle(actual_euler, euler)) {
                    euler = actual_euler;
                }
            }

            auto to_deg = [](math::Vec3 angle) {
                for(usize i = 0; i != 3; ++i) {
                    float a = math::to_deg(angle[i]);
                    if(a < 0.0f) {
                        a += (std::round(a / -360.0f) + 1.0f) * 360.0f;
                    }
                    y_debug_assert(a >= 0.0f);
                    a = std::fmod(a, 360.0f);
                    y_debug_assert(a < 360.0f);
                    if(a > 180.0f) {
                        a -= 360.0f;
                    }
                    angle[i] = a;
                }
                return angle;
            };

            auto to_rad = [](math::Vec3 angle) {
                for(usize i = 0; i != 3; ++i) {
                    angle[i] = math::to_rad(angle[i]);
                }
                return angle;
            };

            math::Vec3 angle = to_deg(euler);
            if(imgui::position_input("##rotation", angle)) {
                angle = to_rad(angle);
                euler = angle;
                rot = math::Quaternion<>::from_euler(angle);
            }
        }

        imgui::table_begin_next_row();

        // scale
        {
            ImGui::TextUnformatted("Scale");
            ImGui::TableNextColumn();


            float scalar_scale = scale.dot(math::Vec3(1.0f / 3.0f));
            if(ImGui::DragFloat("##scale", &scalar_scale, 0.1f, 0.0f, 0.0f, "%.3f")) {
                scale = std::max(0.001f, scalar_scale);
            }

            const bool is_uniform = (scale.max_component() - scale.min_component()) <= 2 * math::epsilon<float>;
            if(!is_uniform) {
                ImGui::SameLine();
                ImGui::TextColored(imgui::error_text_color, ICON_FA_EXCLAMATION_TRIANGLE);
                if(ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(fmt_c_str("Scale is not uniform: %", scale));
                    ImGui::EndTooltip();
                }
            }
        }

        component->set_transform(math::Transform<>(pos, rot, scale));
    }
};

struct EditorComponentWidget : public ComponentPanelWidget<EditorComponentWidget, EditorComponent> {
    core::String component_name() const override {
        return "Entity";
    }

    void on_gui(ecs::EntityId id, EditorComponent* component) {
        if(!ImGui::BeginTable("#properties", 2, table_flags)) {
            return;
        }
        imgui::table_begin_next_row();
        y_defer(ImGui::EndTable());

        const core::String& name = component->name();

        {
            ImGui::TextUnformatted("Name");
            ImGui::TableNextColumn();

            std::array<char, 1024> buffer = {};
            std::copy(name.begin(), name.end(), buffer.begin());

            if(ImGui::InputText("##name", buffer.data(), buffer.size())) {
                component->set_name(buffer.data());
            }
        }

        imgui::table_begin_next_row();

        {
            ImGui::TextUnformatted("Id");
            ImGui::TableNextColumn();

            std::array<char, 32> buffer = {};

            std::snprintf(buffer.data(), buffer.size(), "%08" PRIu32, id.index());
            ImGui::InputText("##id", buffer.data(), buffer.size(), ImGuiInputTextFlags_ReadOnly);

            ImGui::SameLine();

            ImGui::TextDisabled("(?)");
            if(ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("id: %08" PRIu32, id.index());
                ImGui::Text("version: %08" PRIu32, id.version());
                ImGui::EndTooltip();
            }
        }

        /*if(component->is_prefab()) {
            imgui::table_begin_next_row();

            ImGui::TextUnformatted("Parent Prefab");
            ImGui::TableNextColumn();
            imgui::asset_selector(component->parent_prefab(), AssetType::Prefab, "Parent Prefab");
        }*/
    }
};

}

