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
#include "Scene.hpp"
#include <entt/entt.hpp>

int main(int argc, char** argv)
{
    auto registry = std::make_shared<entt::registry>();
    auto& core =NEGUI2::Core::get_instance();
    App::Scene scene(registry);
    NEGUI2::TextureDemo demo2;
    scene.init();

    while(!core.should_close())
    {
        scene.update();
        demo2.update();
        core.update();
    }
    core.wait_idle();
    return EXIT_SUCCESS;

}