#include "System.hpp"

namespace App
{
    System::System(std::shared_ptr<entt::registry> registry)
        : IModule(registry)
    {
    }

    System::~System()
    {
    }

    void System::init()
    {
    }

    void System::update()
    {
    }

}
