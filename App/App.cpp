#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Ui/UiDemo.hpp"
#include "NEGUI2/Ui/PlotDemo.hpp"
#include "NEGUI2/Ui/TextEditDemo.hpp"
#include "NEGUI2/3D/Cube.hpp"
#include <cstdlib>
#include <memory>

int main(int argc, char** argv)
{
    auto& core =NEGUI2::Core::get_instance();
    core.init();

    {
        auto demo_ui = std::make_shared<NEGUI2::UiDemo>();
        core.add_userinterface("ui demo", demo_ui);
    }
    {
        auto plot_demo = std::make_shared<NEGUI2::PlotDemo>();
        core.add_userinterface("plot demo", plot_demo);
    }
    
    {
        auto edit_demo = std::make_shared<NEGUI2::TextEditDemo>();
        core.add_userinterface("edit demo", edit_demo);
    }

    {
        auto cube = std::make_shared<NEGUI2::Cube>();
        core.add_3d_object("cube", cube);
    }

    while(!core.should_colse())
    {
        core.update();
    }
    core.destroy();

    return EXIT_SUCCESS;

}