#ifndef _WIDGET_HPP
#define _WIDGET_HPP
#include "IModule.hpp"
#include <vulkan/vulkan.h>

namespace App
{
    class Widget : public IModule
    {
        VkDescriptorSet texture_id_;
        public:
        Widget(std::shared_ptr<entt::registry> registry);
        virtual ~Widget() override;
        virtual void init() override;
        virtual void update() override;
    };

}
#endif
