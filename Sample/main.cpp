#include <NEGUI2/Core.hpp>
#include <cstdlib>

int main(const int argv, const char** argc)
{
    NEGUI2::Core core;
    core.init();
    while(!core.should_close())
    {
        core.update();
    }
    return EXIT_SUCCESS;
}