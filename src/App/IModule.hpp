#ifndef _IMODULE_HPP
#define _IMODULE_HPP
#include <entt/entt.hpp>
#include <memory>
namespace App
{
    class IModule
    {
    protected:
        std::shared_ptr<entt::registry> registry_;

    public:
        IModule(std::shared_ptr<entt::registry> registry) : registry_(registry) {}
        virtual ~IModule() {}
        virtual void init() = 0;
        virtual void update() = 0;
    };
}

#endif