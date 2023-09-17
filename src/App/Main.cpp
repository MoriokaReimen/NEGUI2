#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Ui/UiDemo.hpp"
#include "NEGUI2/Ui/PlotDemo.hpp"
#include "NEGUI2/Ui/TextEditDemo.hpp"
#include "NEGUI2/Ui/TextureDemo.hpp"
#include "NEGUI2/3D/Triangle.hpp"
#include "NEGUI2/3D/Coordinate.hpp"
#include "NEGUI2/3D/IDisplayObject.hpp"
#include <cstdlib>
#include <memory>
#include "NEGUI2/3D/Camera.hpp"
#include "NEGUI2/3D/Grid.hpp"
#include <spdlog/spdlog.h>

int main(int argc, char** argv)
{
    auto& core =NEGUI2::Core::get_instance();

    NEGUI2::TextureDemo demo2;
    
    auto grid = std::make_shared<NEGUI2::Grid>();
    grid->init();
    core.display_objects.push_back(grid);

    auto triangle = std::make_shared<NEGUI2::Triangle>();
    triangle->init();
    core.display_objects.push_back(triangle);

    auto coord = std::make_shared<NEGUI2::Coordinate>();
    coord->init();
    core.display_objects.push_back(coord);

    NEGUI2::Camera camera;

    double x = 10.0;
    double y = 10.0;
    double z = 10.0;
    while(!core.should_close())
    {

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

        spdlog::info("{} {} {}", x, y, z);
        camera.set_position(Eigen::Vector3d(x, y, z));
        camera.lookat(Eigen::Vector3d::Zero());
        camera.upload();
        demo2.update();
        core.update();
    }
    core.wait_idle();
    return EXIT_SUCCESS;

}