#include "NEGUI2/3D/AABB.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>

namespace NEGUI2
{
    AABB::AABB()
    : BaseTransform(), pipeline_(nullptr), pipeline_layout_(nullptr),
      box_()
    {
    }

    AABB::~AABB()
    {
    }


    void AABB::render(vk::raii::CommandBuffer &command)
    {       
        auto &core = Core::get_instance();

        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        Eigen::Vector3f max = box_.max().cast<float>();
        Eigen::Vector3f min = box_.min().cast<float>();
        auto diff = max - min;
        Eigen::Affine3f offset(Eigen::Translation3f(diff.x() / 2.f, diff.y() / 2.f, diff.z() / 2.f));
        Eigen::Matrix4f scale = Eigen::Scaling(Eigen::Vector4f(diff.x(), diff.y(), diff.z(), 1.f));
        Eigen::Matrix4f mat = scale.matrix() * offset.matrix() * transform_.matrix().cast<float>();
        command.pushConstants<Eigen::Matrix4f>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, mat);
        
        command.draw(24, 1, 0, 0);
    }

    void AABB::set_box(const Eigen::AlignedBox3d &box)
    {
        box_ = box;
    }

    void AABB::init()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto& shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main")
                            .setModule(shader.get("AABB.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main")
                            .setModule(shader.get("AABB.FRAG"));
        }

        vk::PipelineVertexInputStateCreateInfo vertex_input_state;

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.setTopology(vk::PrimitiveTopology::eLineList)
            .setPrimitiveRestartEnable(vk::False);

        vk::PipelineDepthStencilStateCreateInfo depth_stencil;
        depth_stencil.setDepthBoundsTestEnable(vk::True)
                     .setDepthCompareOp(vk::CompareOp::eLess)
                     .setDepthTestEnable(vk::True)
                     .setDepthWriteEnable(vk::False)
                     .setMaxDepthBounds(1.f)
                     .setMinDepthBounds(0.f);

        std::array<vk::Viewport, 1> viewport;
        viewport[0].setX(0.f).setY(0.f).setWidth(extent.width).setHeight(extent.height)
            .setMinDepth(0.f)
            .setMaxDepth(1.f);

        std::array<vk::Rect2D, 1> scissor;
        scissor[0].setOffset({0u, 0u}).setExtent(extent);

        vk::PipelineViewportStateCreateInfo viewport_state;
        viewport_state.setViewports(viewport).setScissors(scissor);

        vk::PipelineRasterizationStateCreateInfo rasterizer;
        rasterizer.setDepthClampEnable(vk::True)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(vk::False);

        vk::PipelineMultisampleStateCreateInfo multisampling;
        multisampling.setSampleShadingEnable(vk::False)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);

        std::array<vk::PipelineColorBlendAttachmentState, 1> colorBlendAttachment;
        colorBlendAttachment[0].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                               .setBlendEnable(vk::True)
                               .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                               .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                               .setColorBlendOp(vk::BlendOp::eAdd)
                               .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setAlphaBlendOp(vk::BlendOp::eMax);

        std::array<float, 4> blend_constant;

        vk::PipelineColorBlendStateCreateInfo color_blending;
        color_blending.setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachments(colorBlendAttachment)
            .setBlendConstants(blend_constant);

        vk::PushConstantRange push_constant;
        push_constant.setStageFlags(vk::ShaderStageFlagBits::eVertex)
                     .setSize(sizeof(Eigen::Matrix4f))
                     .setOffset(0);

        vk::PipelineLayoutCreateInfo pipeline_layout;
        pipeline_layout.setSetLayouts(*core.gpu.descriptor_set_layout)
                       .setPushConstantRanges(push_constant);

        auto &device = core.gpu.device;
        pipeline_layout_ = device.createPipelineLayout(pipeline_layout);

        auto &render_pass = core.off_screen.render_pass;

        vk::GraphicsPipelineCreateInfo pipeline_info;
        pipeline_info.setStages(shader_stages)
            .setPVertexInputState(&vertex_input_state)
            .setPInputAssemblyState(&input_assembly)
            .setPViewportState(&viewport_state)
            .setPRasterizationState(&rasterizer)
            .setPMultisampleState(&multisampling)
            .setPColorBlendState(&color_blending)
            .setLayout(*pipeline_layout_)
            .setRenderPass(*render_pass)
            .setPDepthStencilState(&depth_stencil)
            .setSubpass(0)
            .setBasePipelineHandle(nullptr);

        auto &pipeline_cache = core.gpu.pipeline_cache;
        pipeline_ = device.createGraphicsPipeline(pipeline_cache, pipeline_info);
    }

}
