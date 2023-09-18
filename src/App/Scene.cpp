#include "Scene.hpp"
#include "NEGUI2/3D/AABB.hpp"
#include "NEGUI2/3D/Coordinate.hpp"
#include "NEGUI2/3D/Triangle.hpp"
#include "NEGUI2/3D/Grid.hpp"
#include "NEGUI2/3D/FullShader.hpp"
#include <imgui/backends/imgui_impl_glfw.h>
#include "NEGUI2/Core/Core.hpp"
#include "Widget.hpp"

namespace App
{
    Scene::Scene(std::shared_ptr<entt::registry> registry)
        : IModule(registry)
    {
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
        coord->set_aabb();
        core.display_objects.push_back(coord);

        auto coord2 = std::make_shared<NEGUI2::Coordinate>();
        coord2->init();
        coord2->set_aabb();
        coord2->set_position(Eigen::Vector3d(10.0, 10.0, 10.0));
        core.display_objects.push_back(coord2);

        auto coord3 = std::make_shared<NEGUI2::Coordinate>();
        coord3->init();
        coord3->set_aabb();
        coord3->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.display_objects.push_back(coord3);

        auto arrow = std::make_shared<NEGUI2::FullShader>();
        arrow->init();
        arrow->set_aabb();
        arrow->set_position(Eigen::Vector3d(-10.0, -10.0, 10.0));
        core.display_objects.push_back(arrow);

        camera_.set_position(Eigen::Vector3d(10.0, 10.0, 10.0));
        camera_.lookat(Eigen::Vector3d::Zero());
        camera_.upload();
    }

    void Scene::update()
    {
        handle_camera_();

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