#ifndef _SHADER_HPP
#define _SHADER_HPP
#include <unordered_map>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace NEGUI2
{
    class Shader
    {
        std::unordered_map<std::string, VkShaderModule> shader_modules_;
        VkDevice device_;

    public:
        Shader();
        Shader(VkDevice &device);
        void add_glsl(const std::string &key, const VkShaderStageFlagBits &shader_stage, const std::string &shader_text);
        void add_glsl_from_file(const std::string &key, const VkShaderStageFlagBits &shader_stage, const std::string &glsl_path);
        void add_spv(const std::string &key, const std::vector<unsigned int>&spv_code);
        void add_spv_from_file(const std::string &key, const std::string &spv_path);
        VkShaderModule &get(const std::string& key);

        void destroy();
    };
}

#endif
