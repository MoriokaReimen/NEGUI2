#include "NEGUI2/Core.hpp"
#include "NEGUI2/DemoUI.hpp"
#include <cstdlib>
#include <memory>

int main(int argc, char** argv)
{
    NEGUI2::Core core;
    core.init();

    {
        auto demo_ui = std::make_shared<NEGUI2::DemoUI>();
        core.add_userinterface("demo", demo_ui);
    }

    while(!core.should_colse())
    {
        core.update();
    }
    core.destroy();

    return EXIT_SUCCESS;

}