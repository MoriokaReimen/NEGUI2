#ifndef _IMODULE_HPP
#define _IMODULE_HPP
#include <entt/entt.hpp>

namespace App
{
    class IModule
    {
    protected:
        entt::registry& registry_;
    public:
        IModule(entt::registry& registry) : registry_(registry) {}
        virtual ~IModule() {}
        virtual void init() = 0;
        virtual void update() = 0;
    };
}

#endif