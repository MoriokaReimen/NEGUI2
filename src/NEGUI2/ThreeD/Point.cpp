#include "NEGUI2/ThreeD/Point.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>
#include <algorithm>
#include <spdlog/fmt/bundled/format.h>
namespace
{
    constexpr size_t MAX_POINT(1000000u);
}

namespace NEGUI2
{
    int32_t Point::instance_count_ = 0u;
    Point::Point()
        : BaseTransform(), pipeline_(nullptr), pipeline_layout_(nullptr), push_constant_(),
          point_data_()
    {
        instance_count_++;
        push_constant_.class_id = get_type_id();
        push_constant_.instance_id = instance_count_;
        push_constant_.model = get_transform().matrix().cast<float>();
    }

    Point::~Point()
    {
    }

    void Point::init()
    {
        // TODO 複数いんすタンス対応

        /* Init Vertex buffer */
        auto &core = Core::get_instance();
        std::string memory_name = fmt::format("PointVertex{}", push_constant_.instance_id);
        core.mm.add_memory(memory_name, sizeof(PointData) * MAX_POINT, Memory::TYPE::VERTEX, false);

        /* パイプライン生成 */
        rebuild();
    }

    void Point::destroy()
    {
    }

    void Point::update(vk::raii::CommandBuffer &command)
    {
        push_constant_.model = get_transform().matrix().cast<float>();

        auto &core = Core::get_instance();
        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        std::string memory_name = fmt::format("PointVertex{}", push_constant_.instance_id);
        auto vertex_buffer = core.mm.get_memory(memory_name);
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        command.bindVertexBuffers(0, {vertex_buffer.buffer}, {0});
        command.pushConstants<PushConstant>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, push_constant_);
        command.draw(point_data_.size(), 1, 0, 0);
    }

    void Point::rebuild()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto &shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main").setModule(shader.get("POINT.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main").setModule(shader.get("POINT.FRAG"));
        }
        std::array<vk::VertexInputBindingDescription, 1> binding_description;
        binding_description[0].binding = 0;
        binding_description[0].setBinding(0).setStride(sizeof(PointData)).setInputRate(vk::VertexInputRate::eVertex);

        std::array<vk::VertexInputAttributeDescription, 3> attribute_description;
        attribute_description[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(PointData, position));
        attribute_description[1].setBinding(0).setLocation(1).setFormat(vk::Format::eR32G32B32A32Sfloat).setOffset(offsetof(PointData, color));
        attribute_description[2].setBinding(0).setLocation(2).setFormat(vk::Format::eR32Sfloat).setOffset(offsetof(PointData, diameter));

        vk::PipelineVertexInputStateCreateInfo vertex_input_state;
        vertex_input_state.setVertexBindingDescriptions(binding_description)
            .setVertexAttributeDescriptions(attribute_description);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.setTopology(vk::PrimitiveTopology::ePointList)
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

        std::array<vk::PipelineColorBlendAttachmentState, 2> colorBlendAttachment;
        colorBlendAttachment[0].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                               .setBlendEnable(vk::False)
                               .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                               .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                               .setColorBlendOp(vk::BlendOp::eAdd)
                               .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setDstAlphaBlendFactor(vk::BlendFactor::eOne)
                               .setAlphaBlendOp(vk::BlendOp::eMax);
        colorBlendAttachment[1].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                               .setBlendEnable(vk::False);

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

    int32_t Point::get_type_id()
    {
        auto &id = typeid(Point);
        return static_cast<int32_t>(id.hash_code());
    }

    int32_t Point::get_instance_id()
    {
        return push_constant_.instance_id;
    }

    bool Point::add(const Eigen::Vector3f& position, const Eigen::Vector4f& color, const float& diameter)
    {
        if(point_data_.size() >= MAX_POINT) return false;

        point_data_.push_back({position, color, diameter});
        auto &core = Core::get_instance();
        // TODO更新した部分だけアップ
        std::string memory_name = fmt::format("PointVertex{}", push_constant_.instance_id);
        core.mm.upload_memory(memory_name, point_data_.data(), sizeof(PointData) * point_data_.size());
        return true;
    }

    bool Point::popback()
    {
        if(point_data_.empty()) return false;
        point_data_.pop_back();
        return true;
    }

    Point::PointData Point::get(size_t index) const
    {
        return point_data_[index];
    }

    size_t Point::size() const
    {
        return point_data_.size();
    }
}
