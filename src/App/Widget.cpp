#include "Widget.hpp"
#include "Scene.hpp"
#include <fstream>
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/ThreeD/Coordinate.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>
namespace
{
    void setup_dock()
    {

        // 2. We want our whole dock node to be positioned in the center of the window, so we'll need to calculate that first.
        // The "work area" is the space inside the platform window created by GLFW, SDL, etc. minus the main menu bar if present.
        ImVec2 workPos = ImGui::GetMainViewport()->WorkPos;   // The coordinates of the top-left corner of the work area
        ImVec2 workSize = ImGui::GetMainViewport()->WorkSize; // The dimensions (size) of the work area

        // We'll need to halve the size, then add those resultant values to the top-left corner to get to the middle point on both X and Y.
        ImVec2 workCenter = ImGui::GetMainViewport()->GetWorkCenter();

        // Set the size and position:
        auto size = ImGui::GetMainViewport()->Size;

        // 3. Now we'll need to create our dock node:
        // ImGuiID id = ImGui::GetID("MainWindowGroup"); // The string chosen here is arbitrary (it just gives us something to work with)
        ImGuiID id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        ImGui::DockBuilderRemoveNode(id); // Clear any preexisting layouts associated with the ID we just chose
        ImGui::DockBuilderAddNode(id);    // Create a new dock node to use
        ImVec2 nodePos{workCenter.x - size.x * 0.5f, workCenter.y - size.y * 0.5f};

        // Set the size and position:
        ImGui::DockBuilderSetNodeSize(id, size);
        ImGui::DockBuilderSetNodePos(id, nodePos);
        // 5. Split the dock node to create spaces to put our windows in:

        // Split the dock node in the left direction to create our first docking space. This will be on the left side of the node.
        // (The 0.5f means that the new space will take up 50% of its parent - the dock node.)
        ImGuiID dock3 = ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, 0.1f, nullptr, &id);
        ImGuiID temp = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 0.9f, nullptr, &id);
        // +-----------+
        // |           |
        // |     1     |
        // |           |
        // +-----------+

        // Split the same dock node in the right direction to create our second docking space.
        // At this point, the dock node has two spaces, one on the left and one on the right.
        ImGuiID dock2 = ImGui::DockBuilderSplitNode(temp, ImGuiDir_Right, 0.2f, nullptr, &temp);
        ImGuiID dock1 = ImGui::DockBuilderSplitNode(temp, ImGuiDir_Left, 0.8f, nullptr, &temp);
        // +-----+-----+
        // |     |     |
        // |  1  |  2  |
        // |     |     |
        // +-----+-----+
        //    split ->

        // For our last docking space, we want it to be under the second one but not under the first.
        // Split the second space in the down direction so that we now have an additional space under it.
        //
        // Notice how "dock2" is now passed rather than "id".
        // The new space takes up 50% of the second space rather than 50% of the original dock node.

        // +-----+-----+
        // |     |  2  |  split
        // |  1  +-----+    |
        // |     |  3  |    V
        // +-----+-----+

        // 6. Add windows to each docking space:
        ImGui::DockBuilderDockWindow("Texture Demo", dock1);
        ImGui::DockBuilderDockWindow("Side Panel R", dock2);
        ImGui::DockBuilderDockWindow("Ribbon", dock3);

        // 7. We're done setting up our docking configuration:
        ImGui::DockBuilderFinish(id);
    }
}

namespace App
{
    Widget::Widget(std::shared_ptr<entt::registry> registry)
        : IModule(registry), texture_id_(nullptr), show_coord_input_(false)
    {
    }

    Widget::~Widget()
    {
    }

    void Widget::init()
    {
        Context context;
        registry_->ctx().emplace<Context>(context);
        auto &core = NEGUI2::Core::get_instance();

        texture_id_ = ImGui_ImplVulkan_AddTexture(*core.off_screen.sampler, *core.off_screen.frame.color_buffer_view, VK_IMAGE_LAYOUT_GENERAL);
        ::setup_dock();
    }

    void Widget::update()
    {

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Create"))
                {
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                }
                if (ImGui::MenuItem("Save as.."))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        ImGuiDockNodeFlags dockNodeFlags = ImGuiDockNodeFlags_NoResizeY | ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoDocking | ImGuiDockNodeFlags_NoSplit | ImGuiDockNodeFlags_NoTabBar;
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockNodeFlags);

        /* Ribbon */
        auto size = ImGui::GetMainViewport()->Size;
        static bool open_side = false;
        ImGui::Begin("Ribbon", nullptr, ImGuiWindowFlags_NoScrollbar);
        if (ImGui::Button("Add Coordinate", {size.y * 0.09f, size.y * 0.09f}))
        {
            show_coord_input_ = true;
        }
        ImGui::SameLine();
        ImGui::Button("Measurement\nSetup", {size.y * 0.09f, size.y * 0.09f});
        ImGui::SameLine();
        ImGui::Button("Execute\nMeasurement", {size.y * 0.09f, size.y * 0.09f});
        ImGui::SameLine();
        ImGui::Button("Process\nSetup", {size.y * 0.09f, size.y * 0.09f});
        ImGui::SameLine();
        ImGui::Button("Process", {size.y * 0.09f, size.y * 0.09f});
        ImGui::End();

        /* Scene Window */
        ImGui::Begin("Texture Demo", nullptr);
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        ImGui::Image(texture_id_, ImVec2{viewportPanelSize.x, viewportPanelSize.y});

        auto is_scene_focused = ImGui::IsWindowFocused();
        auto scene_size = ImGui::GetWindowSize();
        auto scene_position = ImGui::GetWindowPos();
        registry_->ctx().get<Context>().is_scene_focused = is_scene_focused;
        registry_->ctx().get<Context>().scene_extent = Eigen::Vector2d(scene_size.x, scene_size.y);
        registry_->ctx().get<Context>().scene_position = Eigen::Vector2d(scene_position.x, scene_position.y);
        ImGui::End();

        /* Performance */
        ImGui::Begin("Side Panel R", nullptr);
        {
            ImGuiContext &g = *GImGui;
            ImGuiIO &io = g.IO;
            ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("%d vertices, %d indices (%d triangles)", io.MetricsRenderVertices, io.MetricsRenderIndices, io.MetricsRenderIndices / 3);
            ImGui::Text("%d visible windows, %d active allocations", io.MetricsRenderWindows, io.MetricsActiveAllocations);
            ImGui::Text("%lf, %lf", scene_size.x, scene_size.y);
            ImGui::Text("%lf, %lf", scene_position.x, scene_position.y);

            {
                auto position = registry_->ctx().get<Scene::Context>().position;
                auto direction = registry_->ctx().get<Scene::Context>().direction;
                ImGui::Text("Position: %lf, %lf, %lf", position.x(), position.y(), position.z());
                ImGui::Text("Direction: %lf, %lf, %lf", direction.x(), direction.y(), direction.z());
            }
        }
        ImGui::End();

        if (show_coord_input_)
        {
            auto &io = ImGui::GetIO();
            auto window_size = io.DisplaySize;
            ImGui::SetNextWindowPos({window_size.x * 0.75f, window_size.y * 0.75f}, ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize({0.f, 0.f});
            static std::shared_ptr<NEGUI2::Coordinate> coord;
            ImGui::Begin("Create Coord", nullptr, ImGuiWindowFlags_NoResize);
            if (coord.get() == nullptr)
            {
                coord = std::make_shared<NEGUI2::Coordinate>();
                coord->init();
                auto &core = NEGUI2::Core::get_instance();
                core.display_objects.push_back(coord);
            }
            static double x = 0.0;
            static double y = 0.0;
            static double z = 0.0;
            ImGui::Text("Center X[mm]");
            ImGui::SameLine();
            ImGui::InputDouble("##Coord X", &x, 0.1);

            ImGui::Text("Center Y[mm]");
            ImGui::SameLine();
            ImGui::InputDouble("##Coord Y", &y, 0.1);

            ImGui::Text("Center Z[mm]");
            ImGui::SameLine();
            ImGui::InputDouble("##Coord Z", &z, 0.1);

            coord->set_position(Eigen::Vector3d(x, y, z));
            if (ImGui::Button("OK"))
            {
                coord = nullptr;
                show_coord_input_ = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("CANCEL"))
            {
                auto &core = NEGUI2::Core::get_instance();
                core.wait_idle();
                core.display_objects.erase(std::remove(std::begin(core.display_objects), std::end(core.display_objects), coord),
                                           std::cend(core.display_objects));
                coord = nullptr;
                show_coord_input_ = false;
            }

            ImGui::End();
        }
    }
}
