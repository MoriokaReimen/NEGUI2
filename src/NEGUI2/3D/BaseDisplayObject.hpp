#ifndef _BASEDISPLAY_OBJECT_HPP
#define _BASEDISPLAY_OBJECT_HPP
#include <cinttypes>
#include <Eigen/Dense>
#include <vulkan/vulkan_raii.hpp>

namespace NEGUI2
{
    struct PushConstant
    {
        uint32_t class_id;
        uint32_t instance_id;
        Eigen::Matrix4f model;
    };
    class BaseDisplayObject
    {
        bool enable_;
    public:
        BaseDisplayObject();
        virtual ~BaseDisplayObject();
        virtual void init() = 0;
        virtual void destroy() = 0;
        virtual void update(vk::raii::CommandBuffer& command) = 0;
        virtual void rebuild() = 0;
        virtual uint32_t get_type_id() = 0;
        virtual uint32_t get_instance_id() = 0;
        virtual bool is_enable() const;
        virtual void set_enabel(const bool enable = true);
    };
}

#endif