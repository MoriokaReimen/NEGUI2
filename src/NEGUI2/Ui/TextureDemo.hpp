#ifndef _TEXTURE_DEMO_HPP
#define _TEXTURE_DEMO_HPP
#include "NEGUI2/Ui/IUserInterface.hpp"
#include <string>
#include <vulkan/vulkan.h>

namespace NEGUI2
{
    class TextureDemo : public IUserInterface
    {
        VkDescriptorSet texture_id_;
    public:
        TextureDemo();
        virtual ~TextureDemo() override;
        virtual void update() override;
    };

}

#endif