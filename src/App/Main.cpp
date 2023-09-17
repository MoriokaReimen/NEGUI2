#include "NEGUI2/Core/Core.hpp"
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include "Scene.hpp"
#include "Widget.hpp"
#include <entt/entt.hpp>

int main(int argc, char** argv)
{
    auto registry = std::make_shared<entt::registry>();
    auto& core =NEGUI2::Core::get_instance();
    App::Scene scene(registry);
    App::Widget widget(registry);
    scene.init();
    widget.init();

    while(!core.should_close())
    {
        scene.update();
        widget.update();
        core.update();
    }
    core.wait_idle();
    return EXIT_SUCCESS;

}