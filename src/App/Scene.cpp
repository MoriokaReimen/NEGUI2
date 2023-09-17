#include "Scene.hpp"
#include "NEGUI2/3D/AABB.hpp"
#include "NEGUI2/3D/Coordinate.hpp"
#include "NEGUI2/3D/Triangle.hpp"
#include "NEGUI2/3D/Grid.hpp"
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
        
        static double x = 10.0;
        static double y = 10.0;
        static double z = 10.0;

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

        auto diff = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 100.0);
        x += diff.x * 0.0001;
        y += diff.y * 0.0001;

        camera_.set_position(Eigen::Vector3d(x, y, z));
        camera_.lookat(Eigen::Vector3d::Zero());
        camera_.upload();
    }

}