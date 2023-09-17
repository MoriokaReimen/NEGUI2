#ifndef _WIDGET_HPP
#define _WIDGET_HPP
#include "IModule.hpp"
#include <vulkan/vulkan.h>
#include <Eigen/Dense>
namespace App
{
    class Widget : public IModule
    {
        VkDescriptorSet texture_id_;
        public:
        struct Context
        {
            bool is_scene_focused;
            Eigen::Vector2d scene_extent;
            Eigen::Vector2d scene_position;
        };

        public:
        Widget(std::shared_ptr<entt::registry> registry);
        virtual ~Widget() override;
        virtual void init() override;
        virtual void update() override;
    };

}
#endif
