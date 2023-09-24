#include "Scene.hpp"
#include "NEGUI2/ThreeD/AABB.hpp"
#include "NEGUI2/ThreeD/Coordinate.hpp"
#include "NEGUI2/ThreeD/Triangle.hpp"
#include "NEGUI2/ThreeD/Grid.hpp"
#include "NEGUI2/ThreeD/FullShader.hpp"
#include <imgui/backends/imgui_impl_glfw.h>
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/ThreeD/BasePickable.hpp"
#include "NEGUI2/ThreeD/Line.hpp"
#include "NEGUI2/ThreeD/Point.hpp"
#include "Widget.hpp"

namespace App
{
    Scene::Scene(std::shared_ptr<entt::registry> registry)
        : IModule(registry)
    {
        Context context;
        context.direction = Eigen::Vector3d::Zero();
        context.position = Eigen::Vector3d::Zero();

        registry_->ctx().emplace<Context>(context);
    }

    Scene::~Scene()
    {
    }

    void Scene::init()
    {
        auto &core = NEGUI2::Core::get_instance();

        auto grid = std::make_shared<NEGUI2::Grid>();
        grid->init();
        core.three_d.add(grid);

        auto triangle = std::make_shared<NEGUI2::Triangle>();
        triangle->init();
        core.three_d.add(triangle);

        auto coord = std::make_shared<NEGUI2::Coordinate>();
        coord->init();
        // coord->set_display_aabb();
        core.three_d.add(coord);

        auto coord2 = std::make_shared<NEGUI2::Coordinate>();
        coord2->init();
        // coord2->set_display_aabb();
        coord2->set_position(Eigen::Vector3d(10.0, 10.0, 10.0));
        core.three_d.add(coord2);

        auto coord3 = std::make_shared<NEGUI2::Coordinate>();
        coord3->init();
        // coord3->set_display_aabb();
        coord3->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.three_d.add(coord3);

#if 0
        auto arrow = std::make_shared<NEGUI2::FullShader>();
        arrow->init();
        // arrow->set_display_aabb();
        arrow->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.three_d.add(arrow);
#endif

        auto line = std::make_shared<NEGUI2::Line>();
        line->init();
        line->add(Eigen::Vector3f(0,0,0), Eigen::Vector3f(0, 0, 10), Eigen::Vector4f(1.f, 0.f, 1.f, 1.f));
        line->add(Eigen::Vector3f(0,0,10), Eigen::Vector3f(0, 20, 10), Eigen::Vector4f(1.f, 0.f, 1.f, 1.f));
        line->add(Eigen::Vector3f(0,20,10), Eigen::Vector3f(10, 20, 10), Eigen::Vector4f(1.f, 0.f, 1.f, 1.f));
        core.three_d.add(line);

        //TODO destroyの実装

        point_ = std::make_shared<NEGUI2::Point>();
        point_->init();
        core.three_d.add(point_);

        target_ = coord;
    }

    void Scene::update()
    {
        handle_camera_();
        auto& core = NEGUI2::Core::get_instance();
        {
            auto color = Eigen::Vector3f::Random();
            point_->add(100 * Eigen::Vector3f::Random(), Eigen::Vector4f(color.x(), color.y(), color.z(), 1.f));
        }

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            return;
        auto mouse = ImGui::GetMousePos();
        auto pos = registry_->ctx().get<Widget::Context>().scene_position;
        auto extent = registry_->ctx().get<Widget::Context>().scene_extent;
        auto uv = Eigen::Vector2d(2.0 * (mouse.x - pos.x()) / extent.x() - 1.0, 2.0 * (mouse.y - pos.y()) / extent.y() - 1.0);

        auto picked = core.three_d.pick(uv);
        if(picked)
        {
            auto before = std::dynamic_pointer_cast<NEGUI2::BasePickable>(target_);
            if(before)
            {
                before->set_display_aabb(false);
            }
            target_ = picked;
            auto after = std::dynamic_pointer_cast<NEGUI2::BasePickable>(target_);
            if(after)
            {
                after->set_display_aabb();
            }
        }
    }

    void Scene::handle_camera_()
    {
        auto wideget_context = registry_->ctx().get<Widget::Context>();
        // TODO CameraのExtentの扱い
        if (!wideget_context.is_scene_focused)
            return;

        static Eigen::Vector3d position(10, 10, 10);
        auto &core = NEGUI2::Core::get_instance();
        auto up = core.three_d.camera().up();
        auto right = core.three_d.camera().right();
        auto front = core.three_d.camera().front();

        // TODO Cameraのsetポジションのバグ

#if 0
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_K)))
            z += 1.0;
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_J)))
            z -= 1.0;

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_W)))
            y += 1.0;
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
            y -= 1.0;

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_A)))
            x += 1.0;
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_D)))
            x -= 1.0;

#endif
        auto diff = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.0);
        position += diff.x * 0.1 * right;
        position += diff.y * 0.1 * up;
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

        position += ImGui::GetIO().MouseWheel * front;

        core.three_d.camera().set_position(position);
        core.three_d.camera().upload();

        auto transform = std::dynamic_pointer_cast<NEGUI2::BaseTransform>(target_);
        if (transform)
        {   
            auto pos = transform->get_position();
            core.three_d.camera().lookat(pos);
        }
    }

}