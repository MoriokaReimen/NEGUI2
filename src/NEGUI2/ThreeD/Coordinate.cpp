#include "NEGUI2/ThreeD/Coordinate.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>

namespace NEGUI2
{
    int32_t Coordinate::instance_count_ = 0u;
    Coordinate::Coordinate()
        : BaseTransform(), BasePickable(), pipeline_(nullptr), pipeline_layout_(nullptr), push_constant_()
    {
        instance_count_++;
        push_constant_.class_id = get_type_id();
        push_constant_.instance_id = instance_count_;
        push_constant_.model = get_transform().matrix().cast<float>();
    }

    Coordinate::~Coordinate()
    {
    }

    void Coordinate::init()
    {
        /* Init aabb */
        box_ = Eigen::AlignedBox3d(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(1.0, 1.0, 1.0));

        /* Init Vertex buffer */
        {
            vertex_data_[0] = Eigen::Vector3f::Zero();
            vertex_data_[1] = Eigen::Vector3f::UnitX();
            vertex_data_[2] = Eigen::Vector3f::Zero();
            vertex_data_[3] = Eigen::Vector3f::UnitY();
            vertex_data_[4] = Eigen::Vector3f::Zero();
            vertex_data_[5] = Eigen::Vector3f::UnitZ();

            auto &core = Core::get_instance();
            core.mm.add_memory("CoordinateVertex", sizeof(Eigen::Vector3f) * vertex_data_.size(), Memory::TYPE::VERTEX, false);
            core.mm.upload_memory("CoordinateVertex", vertex_data_.data(), sizeof(Eigen::Vector3f) * vertex_data_.size());
        }

        /* Init Color Data */
        {
            color_data_[0] = Eigen::Vector4f(1.f, 0.f, 0.f, 1.f);
            color_data_[1] = Eigen::Vector4f(1.f, 0.f, 0.f, 1.f);

            color_data_[2] = Eigen::Vector4f(0.f, 1.f, 0.f, 1.f);
            color_data_[3] = Eigen::Vector4f(0.f, 1.f, 0.f, 1.f);

            color_data_[4] = Eigen::Vector4f(0.f, 0.f, 1.f, 1.f);
            color_data_[5] = Eigen::Vector4f(0.f, 0.f, 1.f, 1.f);

            auto &core = Core::get_instance();
            core.wait_idle();
            core.mm.add_memory("CoordinateColor", sizeof(Eigen::Vector4f) * color_data_.size(), Memory::TYPE::VERTEX, false);
            core.mm.upload_memory("CoordinateColor", color_data_.data(), sizeof(Eigen::Vector4f) * color_data_.size());
        }

        /* パイプライン生成 */
        rebuild();
    }

    void Coordinate::destroy()
    {
    }

    void Coordinate::update(vk::raii::CommandBuffer &command)
    {
        push_constant_.model = get_transform().matrix().cast<float>();

        auto &core = Core::get_instance();

        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        auto vertex_buffer = core.mm.get_memory("CoordinateVertex");
        auto color_buffer = core.mm.get_memory("CoordinateColor");
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        command.bindVertexBuffers(0, {vertex_buffer.buffer, color_buffer.buffer}, {0, 0});
        command.pushConstants<PushConstant>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, push_constant_);

        command.draw(vertex_data_.size(), 1, 0, 0);
    }

    void Coordinate::rebuild()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto &shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main").setModule(shader.get("BASE.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main").setModule(shader.get("BASE.FRAG"));
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

        std::array<vk::PipelineColorBlendAttachmentState, 2> colorBlendAttachment;
        colorBlendAttachment[0].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA).setBlendEnable(vk::False);
        colorBlendAttachment[1].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                               .setBlendEnable(vk::False);

        std::array<float, 4> blend_constant{};

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

    int32_t Coordinate::get_type_id()
    {
        auto &id = typeid(Coordinate);
        return static_cast<int32_t>(id.hash_code());
    }

    int32_t Coordinate::get_instance_id()
    {
        return push_constant_.instance_id;
    }

    double Coordinate::pick(const Eigen::Vector3d &origin, const Eigen::Vector3d &direction)
    {
        const Eigen::Vector3d position = get_position();
        auto diff = position - origin;
        
        auto dot = diff.dot(direction);
        if(dot < 0.0) return -1.0;

        auto L = dot * direction;
        auto dist = L.cross(diff).norm() / L.norm();
        auto ret = diff.squaredNorm();
        if (dist > 1.0)
        {
            ret = -1.0;
        }

        return ret;
    }
}
