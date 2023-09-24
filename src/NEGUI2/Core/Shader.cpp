#include <NEGUI2/Core/Shader.hpp>
#include <fstream>
#include <SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <spdlog/spdlog.h>
#include "NEGUI2/Core/Core.hpp"

namespace
{
  EShLanguage translateShaderStage(VkShaderStageFlagBits stage)
  {
    switch (stage)
    {
    case VK_SHADER_STAGE_VERTEX_BIT:
      return EShLangVertex;
    case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
      return EShLangTessControl;
    case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
      return EShLangTessEvaluation;
    case VK_SHADER_STAGE_GEOMETRY_BIT:
      return EShLangGeometry;
    case VK_SHADER_STAGE_FRAGMENT_BIT:
      return EShLangFragment;
    case VK_SHADER_STAGE_COMPUTE_BIT:
      return EShLangCompute;
    case VK_SHADER_STAGE_RAYGEN_BIT_NV:
      return EShLangRayGenNV;
    case VK_SHADER_STAGE_ANY_HIT_BIT_NV:
      return EShLangAnyHitNV;
    case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:
      return EShLangClosestHitNV;
    case VK_SHADER_STAGE_MISS_BIT_NV:
      return EShLangMissNV;
    case VK_SHADER_STAGE_INTERSECTION_BIT_NV:
      return EShLangIntersectNV;
    case VK_SHADER_STAGE_CALLABLE_BIT_NV:
      return EShLangCallableNV;
    case VK_SHADER_STAGE_TASK_BIT_NV:
      return EShLangTaskNV;
    case VK_SHADER_STAGE_MESH_BIT_NV:
      return EShLangMeshNV;
    default:
      assert(false && "Unknown shader stage");
      return EShLangVertex;
    }
  }

  bool GLSLtoSPV(const VkShaderStageFlagBits shaderType, std::string const &glslShader, std::vector<unsigned int> &spvShader)
  {
    EShLanguage stage = translateShaderStage(shaderType);

    const char *shaderStrings[1];
    shaderStrings[0] = glslShader.data();

    glslang::TShader shader(stage);
    shader.setStrings(shaderStrings, 1);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(GetDefaultResources(), 100, false, messages))
    {
      spdlog::error("{}", shader.getInfoLog());
      spdlog::error("{}", shader.getInfoDebugLog());
      return false; // something didn't work
    }

    glslang::TProgram program;
    program.addShader(&shader);

    //
    // Program-level processing...
    //

    if (!program.link(messages))
    {
      spdlog::error("{}", shader.getInfoLog());
      spdlog::error("{}", shader.getInfoDebugLog());
      return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
    return true;
  }

  VkShaderModule createShaderModule(VkDevice const &device, VkShaderStageFlagBits shaderStage, std::string const &shaderText)
  {
    std::vector<unsigned int> spv_code;
    if (!GLSLtoSPV(shaderStage, shaderText, spv_code))
    {
      throw std::runtime_error("Could not convert glsl shader to spir-v -> terminating");
    }
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = spv_code.size();
    info.pCode = reinterpret_cast<const uint32_t *>(spv_code.data());
    VkShaderModule shader_module;
    vkCreateShaderModule(device, &info, nullptr, &shader_module);

    return shader_module;
  }

  VkShaderModule makeShaderModule(VkDevice const &device, VkShaderStageFlagBits shaderStage, std::string const &shaderText)
  {
    std::vector<unsigned int> spv_code;
    if (!::GLSLtoSPV(shaderStage, shaderText, spv_code))
    {
      throw std::runtime_error("Could not convert glsl shader to spir-v -> terminating");
    }
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = spv_code.size();
    info.pCode = reinterpret_cast<const uint32_t *>(spv_code.data());
    VkShaderModule shader_module;
    vkCreateShaderModule(device, &info, nullptr, &shader_module);

    return shader_module;
  }
}

namespace NEGUI2
{
  Shader::Shader()
      : shader_modules_()
  {
  }

  void Shader::init()
  {
    add_spv_from_file("BASE.VERT", "./shader/Base.vert.spv");
    add_spv_from_file("BASE.FRAG", "./shader/Base.frag.spv");
    add_spv_from_file("GRID.VERT", "./shader/Grid.vert.spv");
    add_spv_from_file("GRID.FRAG", "./shader/Grid.frag.spv");
    add_spv_from_file("AABB.VERT", "./shader/AABB.vert.spv");
    add_spv_from_file("AABB.FRAG", "./shader/AABB.frag.spv");
    add_spv_from_file("FULLSHADER.VERT", "./shader/FullShader.vert.spv");
    add_spv_from_file("FULLSHADER.FRAG", "./shader/FullShader.frag.spv");
    add_spv_from_file("LINE.VERT", "./shader/Line.vert.spv");
    add_spv_from_file("LINE.FRAG", "./shader/Line.frag.spv");
  }

  void Shader::add_glsl(const std::string &key, const VkShaderStageFlagBits &shader_stage, const std::string &shader_text)
  {
    auto &device = Core::get_instance().gpu.device;
    glslang::InitializeProcess();
    VkShaderModule shader_module = ::makeShaderModule(*device, shader_stage, shader_text);
    glslang::FinalizeProcess();
    shader_modules_.emplace(key, shader_module);
  }

  void Shader::add_spv(const std::string &key, const std::vector<unsigned int> &spv_code)
  {
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = sizeof(unsigned int) * spv_code.size();
    info.pCode = reinterpret_cast<const uint32_t *>(spv_code.data());
    VkShaderModule shader_module;
    auto &device = Core::get_instance().gpu.device;
    vkCreateShaderModule(*device, &info, nullptr, &shader_module);
    shader_modules_.emplace(key, std::move(shader_module));
  }

  void Shader::add_glsl_from_file(const std::string &key, const VkShaderStageFlagBits &shader_stage, const std::string &glsl_path)
  {
    /* ファイルをテキストに読み込み */
    std::ifstream file(glsl_path);
    if (!file.is_open())
    {
      throw std::runtime_error("failed to open file!");
    }
    std::string glsl_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    add_glsl(key, shader_stage, glsl_code);
  }

  void Shader::add_spv_from_file(const std::string &key, const std::string &spv_path)
  {
    /* ファイルをテキストに読み込み */
    std::ifstream file(spv_path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
      throw std::runtime_error("failed to open file!");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<unsigned int> spv_code(fileSize / sizeof(unsigned int));
    file.seekg(0);
    file.read(reinterpret_cast<char *>(spv_code.data()), fileSize);
    file.close();
    add_spv(key, spv_code);
  }

  VkShaderModule &Shader::get(const std::string &key)
  {
    return shader_modules_.at(key);
  }

  void Shader::destroy()
  {
    for (auto &shader_module : shader_modules_)
    {
      auto &device = Core::get_instance().gpu.device;
      vkDestroyShaderModule(*device, shader_module.second, nullptr);
    }
  }
}
