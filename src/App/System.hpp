#ifndef _SYSTEM_HPP
#define _SYSTEM_HPP
#include "IModule.hpp"

namespace App
{
    class System : public IModule
    {
        public:
        System(std::shared_ptr<entt::registry> registry);
        virtual ~System() override;
        virtual void init() = 0;
        virtual void update() = 0;
    };

}
#endif
