#ifndef _I3D_OBJECT_HPP
#define _I3D_OBJECT_HPP
#include <volk.h>
namespace NEGUI2
{
    struct DeviceData;
    struct WindowData;
    class I3DObject
    {

    public:
        I3DObject() {}
        virtual ~I3DObject() {}
        virtual void init(DeviceData& device_data) = 0;
        virtual void create_pipeline(DeviceData& device_data, WindowData& window_data) = 0;
        virtual void destroy() = 0;
        virtual void update(VkCommandBuffer& command) = 0;
    };
}

#endif