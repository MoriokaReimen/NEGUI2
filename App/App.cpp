#include "NEGUI2/Core.hpp"
#include <cstdlib>

int main(int argc, char** argv)
{
    NEGUI2::Core core;
    core.init();
    core.destroy();

    return EXIT_SUCCESS;

}