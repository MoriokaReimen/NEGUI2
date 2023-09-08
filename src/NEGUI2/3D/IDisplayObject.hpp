#ifndef _IDISPLAY_OBJECT_HPP
#define _IDISPLAY_OBJECT_HPP
#include <cinttypes>
#include <Eigen/Dense>
#include <vulkan/vulkan_raii.hpp>

namespace NEGUI2
{
    class IDisplayObject
    {
    public:
        IDisplayObject();
        virtual ~IDisplayObject();
        virtual void init() = 0;
        virtual void destroy() = 0;
        virtual void update(vk::raii::CommandBuffer& command) = 0;

        virtual uint32_t get_type_id() = 0;
        virtual uint32_t get_instance_id() = 0;
    };
}

#endif