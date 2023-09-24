#ifndef _THREED_HPP
#define _THREED_HPP
#include <memory>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <Eigen/Dense>
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/Camera.hpp"
#include "NEGUI2/ThreeD/AABB.hpp"
#include <optional>

namespace NEGUI2
{
    class ThreeD
    {
        std::vector<std::shared_ptr<BaseDisplayObject>> display_objects_;
        Camera camera_;
        AABB aabb_;

    public:
        ThreeD();
        ~ThreeD();

        void init();

        void update(vk::raii::CommandBuffer &command_buffer);
        std::shared_ptr<BaseDisplayObject> pick(const Eigen::Vector2d &uv);

        void add(std::shared_ptr<BaseDisplayObject> display_object);
        std::optional<size_t> peek(std::shared_ptr<BaseDisplayObject> display_object);
        std::shared_ptr<BaseDisplayObject> get_display_object(const size_t &index);

        void erase(const size_t &index);
        void erase(std::shared_ptr<BaseDisplayObject> display_object);

        Camera &camera();
        const Camera &camera() const;
    };
}

#endif