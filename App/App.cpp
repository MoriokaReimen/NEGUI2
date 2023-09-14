#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Ui/UiDemo.hpp"
#include "NEGUI2/Ui/PlotDemo.hpp"
#include "NEGUI2/Ui/TextEditDemo.hpp"
#include "NEGUI2/Ui/TextureDemo.hpp"
#include "NEGUI2/3D/Triangle.hpp"
#include "NEGUI2/3D/IDisplayObject.hpp"
#include <cstdlib>
#include <memory>
#include "NEGUI2/3D/Camera.hpp"

int main(int argc, char** argv)
{
    auto& core =NEGUI2::Core::get_instance();

    NEGUI2::TextureDemo demo2;
    NEGUI2::PlotDemo demo;
    NEGUI2::Camera camera;
    auto triangle = std::make_shared<NEGUI2::Triangle>();
    triangle->init();
    core.display_objects.push_back(triangle);
    int pos = 0u;

    while(!core.should_close())
    {
        pos = pos < 10000 ? pos + 1 : -5000u;
        camera.set_mouse(pos, pos);
        camera.upload();
        demo.update();
        demo2.update();
        core.update();
    }
    core.wait_idle();
    return EXIT_SUCCESS;

}