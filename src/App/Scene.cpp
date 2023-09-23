#include "Scene.hpp"
#include "NEGUI2/3D/AABB.hpp"
#include "NEGUI2/3D/Coordinate.hpp"
#include "NEGUI2/3D/Triangle.hpp"
#include "NEGUI2/3D/Grid.hpp"
#include "NEGUI2/3D/FullShader.hpp"
#include <imgui/backends/imgui_impl_glfw.h>
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/3D/BasePickable.hpp"
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
        core.display_objects.push_back(grid);

        auto triangle = std::make_shared<NEGUI2::Triangle>();
        triangle->init();
        core.display_objects.push_back(triangle);

        auto coord = std::make_shared<NEGUI2::Coordinate>();
        coord->init();
        // coord->set_display_aabb();
        core.display_objects.push_back(coord);

        auto coord2 = std::make_shared<NEGUI2::Coordinate>();
        coord2->init();
        // coord2->set_display_aabb();
        coord2->set_position(Eigen::Vector3d(10.0, 10.0, 10.0));
        core.display_objects.push_back(coord2);

        auto coord3 = std::make_shared<NEGUI2::Coordinate>();
        coord3->init();
        // coord3->set_display_aabb();
        coord3->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.display_objects.push_back(coord3);

#if 0
        auto arrow = std::make_shared<NEGUI2::FullShader>();
        arrow->init();
        // arrow->set_display_aabb();
        arrow->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.display_objects.push_back(arrow);
#endif
        camera_.set_position(Eigen::Vector3d(10.0, 10.0, 10.0));
        camera_.lookat(Eigen::Vector3d::Zero());
        camera_.upload();
    }

    void Scene::update()
    {
        handle_camera_();

        auto &core = NEGUI2::Core::get_instance();
        for (auto display_object : core.display_objects)
        {
            auto pickable = std::dynamic_pointer_cast<NEGUI2::BasePickable>(display_object);
            auto mouse = ImGui::GetMousePos();
            auto pos = registry_->ctx().get<Widget::Context>().scene_position;
            auto extent = registry_->ctx().get<Widget::Context>().scene_extent;
            auto uv = Eigen::Vector2d(2.0 * (mouse.x- pos.x())/ extent.x() - 1.0, 2.0 * (mouse.y- pos.y())/ extent.y() - 1.0);
            
            auto origin = camera_.uv_to_near_xyz(uv);
            auto direction = camera_.uv_to_direction(uv);

            if (pickable)
            {
                auto dist = pickable->pick(origin, direction);
                registry_->ctx().get<Context>().position = origin;
                registry_->ctx().get<Context>().direction = direction;
                if(0.0 < dist)
                {
                    pickable->toggle_display_aabb();
                }
            }
        }
    }

    void Scene::handle_camera_()
    {
        auto wideget_context = registry_->ctx().get<Widget::Context>();
        //TODO CameraのExtentの扱い
        if(!wideget_context.is_scene_focused) return;
        
        static Eigen::Vector3d position(10, 10, 10);
        auto up = camera_.up();
        auto right = camera_.right();
        auto front = camera_.front();

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

        camera_.set_position(position);
        camera_.lookat(Eigen::Vector3d::Zero());
        camera_.upload();
    }

}