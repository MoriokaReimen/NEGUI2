#include "NEGUI2/Core.hpp"
#include "NEGUI2/UiDemo.hpp"
#include "NEGUI2/PlotDemo.hpp"
#include <cstdlib>
#include <memory>

int main(int argc, char** argv)
{
    NEGUI2::Core core;
    core.init();

    {
        auto demo_ui = std::make_shared<NEGUI2::UiDemo>();
        core.add_userinterface("ui demo", demo_ui);
    }
    {
        auto plot_demo = std::make_shared<NEGUI2::PlotDemo>();
        core.add_userinterface("plot demo", plot_demo);
    }

    while(!core.should_colse())
    {
        core.update();
    }
    core.destroy();

    return EXIT_SUCCESS;

}