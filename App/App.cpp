#include "NEGUI2/Core.hpp"
#include <cstdlib>

int main(int argc, char** argv)
{
    NEGUI2::Core core;
    core.init();
    while(!core.should_colse())
    {
        core.update();
    }
    core.destroy();

    return EXIT_SUCCESS;

}