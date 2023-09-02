#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Ui/UiDemo.hpp"
#include "NEGUI2/Ui/PlotDemo.hpp"
#include "NEGUI2/Ui/TextEditDemo.hpp"
#include "NEGUI2/Ui/TextureDemo.hpp"
#include <cstdlib>
#include <memory>

int main(int argc, char** argv)
{
    auto& core =NEGUI2::Core::get_instance();

    NEGUI2::TextureDemo demo2;
    NEGUI2::PlotDemo demo;

    while(!core.should_close())
    {
        demo.update();
        demo2.update();
        core.update();
    }
    core.wait_idle();
    return EXIT_SUCCESS;

}