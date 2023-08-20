#ifndef _CUBE_HPP
#define _CUBE_HPP
#include "NEGUI2/3D/I3DObject.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
namespace NEGUI2
{
    class Cube : public I3DObject
    {
        VkDevice device_;
        VmaAllocator allocator_;
        
        VkPipeline pipeline_;
        VkPipelineLayout pipeline_layout_;

        VkBuffer vertex_buffer_;
        VmaAllocation vertex_allocation_;

        VkBuffer index_buffer_;
        VmaAllocation index_allocation_;

    public:
        Cube();
        ~Cube() override;
        void init(DeviceData& device_data) override;
        void create_pipeline(DeviceData& device_data, WindowData& window_data) override;
        void destroy() override;
        void update(VkCommandBuffer& command) override;
    };
}

#endif