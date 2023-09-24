#include "NEGUI2/ThreeD/Line.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>
#include <algorithm>
namespace
{
    constexpr size_t MAX_LINE(10000u);
}

namespace NEGUI2
{
    uint32_t Line::instance_count_ = 0u;
    Line::Line()
        : BaseTransform(), pipeline_(nullptr), pipeline_layout_(nullptr), push_constant_(),
          line_data_()
    {
        instance_count_++;
        push_constant_.class_id = get_type_id();
        push_constant_.instance_id = instance_count_;
        push_constant_.model = get_transform().matrix().cast<float>();
    }

    Line::~Line()
    {
    }

    void Line::init()
    {
        // TODO 複数いんすタンス対応

        /* Init Vertex buffer */
        auto &core = Core::get_instance();
        core.mm.add_memory("LineVertex", sizeof(PointData) * MAX_LINE, Memory::TYPE::VERTEX, false);

        /* パイプライン生成 */
        rebuild();
    }

    void Line::destroy()
    {
    }

    void Line::update(vk::raii::CommandBuffer &command)
    {
        push_constant_.model = get_transform().matrix().cast<float>();

        auto &core = Core::get_instance();
        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        auto vertex_buffer = core.mm.get_memory("LineVertex");
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        command.bindVertexBuffers(0, {vertex_buffer.buffer}, {0});
        command.pushConstants<PushConstant>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, push_constant_);
        command.draw(2, line_data_.size(), 0, 0);
    }

    void Line::rebuild()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto &shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main").setModule(shader.get("LINE.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main").setModule(shader.get("LINE.FRAG"));
        }
        std::array<vk::VertexInputBindingDescription, 1> binding_description;
        binding_description[0].binding = 0;
        binding_description[0].setBinding(0).setStride(sizeof(PointData)).setInputRate(vk::VertexInputRate::eInstance);

        std::array<vk::VertexInputAttributeDescription, 4> attribute_description;
        attribute_description[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(PointData, start));
        attribute_description[1].setBinding(0).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(PointData, end));
        attribute_description[2].setBinding(0).setLocation(2).setFormat(vk::Format::eR32G32B32A32Sfloat).setOffset(offsetof(PointData, color));
        attribute_description[3].setBinding(0).setLocation(3).setFormat(vk::Format::eR32Sfloat).setOffset(offsetof(PointData, diameter));

        vk::PipelineVertexInputStateCreateInfo vertex_input_state;
        vertex_input_state.setVertexBindingDescriptions(binding_description)
            .setVertexAttributeDescriptions(attribute_description);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.setTopology(vk::PrimitiveTopology::eLineList)
            .setPrimitiveRestartEnable(vk::False);

        vk::PipelineDepthStencilStateCreateInfo depth_stencil;
        depth_stencil.setDepthBoundsTestEnable(vk::False)
            .setDepthCompareOp(vk::CompareOp::eLess)
            .setDepthTestEnable(vk::True)
            .setDepthWriteEnable(vk::True)
            .setMaxDepthBounds(1.f)
            .setMinDepthBounds(0.f);

        std::array<vk::Viewport, 1> viewport;
        viewport[0].setX(0.f).setY(0.f).setWidth(extent.width).setHeight(extent.height).setMinDepth(0.f).setMaxDepth(1.f);

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
                               .setBlendEnable(vk::False)
                               .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                               .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                               .setColorBlendOp(vk::BlendOp::eAdd)
                               .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setAlphaBlendOp(vk::BlendOp::eMax);

        std::array<float, 4> blend_constant{0};

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

    uint32_t Line::get_type_id()
    {
        auto &id = typeid(Line);
        return static_cast<uint32_t>(id.hash_code());
    }

    uint32_t Line::get_instance_id()
    {
        return push_constant_.instance_id;
    }

    bool Line::add(const Eigen::Vector3f& start, const Eigen::Vector3f& end, const Eigen::Vector4f& color, const float& diameter)
    {
        if(line_data_.size() >= MAX_LINE) return false;

        line_data_.push_back({start, end, color, diameter});
        auto &core = Core::get_instance();
        core.mm.upload_memory("LineVertex", line_data_.data(), sizeof(PointData) * line_data_.size());
        return true;
    }

    bool Line::popback()
    {
        if(line_data_.empty()) return false;
        line_data_.pop_back();
        return true;
    }

    Line::PointData Line::get(size_t index) const
    {
        return line_data_[index];
    }

    size_t Line::size() const
    {
        return line_data_.size();
    }
}
