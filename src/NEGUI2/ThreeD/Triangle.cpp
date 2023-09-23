#include "NEGUI2/ThreeD/Triangle.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>

namespace NEGUI2
{
    uint32_t Triangle::instance_count_ = 0u;
    Triangle::Triangle()
    : BaseTransform(), pipeline_(nullptr), pipeline_layout_(nullptr)
    {
        instance_count_++;
        push_constant_.class_id = get_type_id();
        push_constant_.instance_id = instance_count_;
        push_constant_.model = get_transform().matrix().cast<float>();
    }

    Triangle::~Triangle()
    {
    }

    void Triangle::init()
    {
        instance_count_++;
        instance_id_ = instance_count_;

        /* Init Vertex buffer */
        {
            vertex_data_[0] = 0.5f * Eigen::Vector3f::UnitY();
            vertex_data_[1] = 0.5f * Eigen::Vector3f::UnitX();
            vertex_data_[2] = -0.5f * Eigen::Vector3f::UnitX();
            auto &core = Core::get_instance();
            core.mm.add_memory("TriangleVertex",
                                                 sizeof(Eigen::Vector3f) * vertex_data_.size(),
                                                 Memory::TYPE::VERTEX);
            core.mm.upload_memory("TriangleVertex", vertex_data_.data(), sizeof(Eigen::Vector3f) * vertex_data_.size());
        }

        /* Init Color buffer */
        {
            color_data_[0] = Eigen::Vector4f(1.f, 0.f, 0.f, 1.f);
            color_data_[1] = Eigen::Vector4f(0.f, 1.f, 0.f, 1.f);
            color_data_[2] = Eigen::Vector4f(0.f, 0.f, 1.f, 1.f);

            auto &core = Core::get_instance();
            core.mm.add_memory("TriangleColor",
                               sizeof(Eigen::Vector4f) * color_data_.size(),
                               Memory::TYPE::VERTEX);
            core.mm.upload_memory("TriangleColor", color_data_.data(), sizeof(Eigen::Vector4f) * color_data_.size());
        }

        /* パイプライン生成 */
        rebuild();
    }

    void Triangle::destroy()
    {
    }

    void Triangle::update(vk::raii::CommandBuffer &command)
    {       
        push_constant_.model = get_transform().matrix().cast<float>();
        
        auto &core = Core::get_instance();

        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        auto vertex_buffer = core.mm.get_memory("TriangleVertex");
        auto color_buffer = core.mm.get_memory("TriangleColor");
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        command.bindVertexBuffers(0, {vertex_buffer.buffer, color_buffer.buffer}, {0, 0});
        command.pushConstants<PushConstant>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, push_constant_);
        
        command.draw(vertex_data_.size(), 1, 0, 0);
    }

    void Triangle::rebuild()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto& shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main")
                            .setModule(shader.get("BASE.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main")
                            .setModule(shader.get("BASE.FRAG"));
        }
        std::array<vk::VertexInputBindingDescription, 2> binding_description;
        binding_description[0].binding = 0;
        binding_description[0].setBinding(0).setStride(sizeof(Eigen::Vector3f)).setInputRate(vk::VertexInputRate::eVertex);
        binding_description[1].binding = 1;
        binding_description[1].setBinding(1).setStride(sizeof(Eigen::Vector4f)).setInputRate(vk::VertexInputRate::eVertex);

        std::array<vk::VertexInputAttributeDescription, 2> attribute_description;
        attribute_description[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);
        attribute_description[1].setBinding(1).setLocation(1).setFormat(vk::Format::eR32G32B32A32Sfloat);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state;
        vertex_input_state.setVertexBindingDescriptions(binding_description)
            .setVertexAttributeDescriptions(attribute_description);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.setTopology(vk::PrimitiveTopology::eTriangleFan)
            .setPrimitiveRestartEnable(vk::False);

        vk::PipelineDepthStencilStateCreateInfo depth_stencil;
        depth_stencil.setDepthBoundsTestEnable(vk::False)
                     .setDepthCompareOp(vk::CompareOp::eLess)
                     .setDepthTestEnable(vk::True)
                     .setDepthWriteEnable(vk::True)
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
        rasterizer.setDepthClampEnable(vk::False)
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
                               .setBlendEnable(vk::False);

        std::array<float, 4> blend_constant;

        vk::PipelineColorBlendStateCreateInfo color_blending;
        color_blending.setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachments(colorBlendAttachment)
            .setBlendConstants(blend_constant);

        vk::PushConstantRange push_constant;
        push_constant.setStageFlags(vk::ShaderStageFlagBits::eVertex)
            .setSize(sizeof(PushConstant))
            .setOffset(0);

        vk::PipelineLayoutCreateInfo pipeline_layout;
        pipeline_layout.setSetLayouts(*core.gpu.descriptor_set_layout)
                       .setPushConstantRanges(push_constant);
        // TODO push constnatの実装

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

    uint32_t Triangle::get_type_id()
    {
        auto &id = typeid(Triangle);
        return static_cast<uint32_t>(id.hash_code());
    }

    uint32_t Triangle::get_instance_id()
    {
        return instance_id_;
    }
}
