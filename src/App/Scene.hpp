#ifndef _SCENE_HPP
#define _SCENE_HPP
#include "IModule.hpp"

namespace App
{
    class Scene : public IModule
    {
        public:
        Scene(entt::registry& registry);
        virtual ~Scene() override;
        virtual void init() = 0;
        virtual void update() = 0;
    };

}
#endif