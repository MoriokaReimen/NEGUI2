#ifndef _WIDGET_HPP
#define _WIDGET_HPP
#include "IModule.hpp"

namespace App
{
    class Widget : public IModule
    {
        public:
        Widget(entt::registry& registry);
        virtual ~Widget() override;
        virtual void init() = 0;
        virtual void update() = 0;
    };

}
#endif
