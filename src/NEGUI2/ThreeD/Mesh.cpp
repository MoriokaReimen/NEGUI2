#include "NEGUI2/ThreeD/Mesh.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/Shader.hpp"
#include <typeinfo>
#include <spdlog/fmt/bundled/format.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace NEGUI2
{
    int32_t Mesh::instance_count_ = 0u;
    Mesh::Mesh()
        : BaseTransform(), pipeline_(nullptr), pipeline_layout_(nullptr),
          vertex_data_(), normal_data_(), indices_(), color_data_()
    {
        Mesh::instance_count_++;
        push_constant_.class_id = get_type_id();
        push_constant_.instance_id = Mesh::instance_count_;
        push_constant_.model = get_transform().matrix().cast<float>();
    }

    Mesh::~Mesh()
    {
    }

    void Mesh::load(const std::filesystem::path& path)
    {
        Assimp::Importer importer;
        auto* scene = importer.ReadFile(path.string(), aiProcess_Triangulate );
        auto* node = scene->mRootNode;
        auto* mesh = scene->mMeshes[0];

        vertex_data_.clear();
        normal_data_.clear();
        indices_.clear();

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Eigen::Vector3f vertex( mesh->mVertices[i].x,  mesh->mVertices[i].y,  mesh->mVertices[i].z);
            vertex_data_.push_back(vertex);
            
            Eigen::Vector3f normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            normal_data_.push_back(normal);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices_.push_back(face.mIndices[j]);
        }

        init();
    }

    void Mesh::init()
    {
        Eigen::Vector3f min = vertex_data_[0];
        Eigen::Vector3f max = vertex_data_[0];
        {
            float min_x = vertex_data_[0].x();
            float min_y = vertex_data_[0].y();
            float min_z = vertex_data_[0].z();

            float max_x = vertex_data_[0].x();
            float max_y = vertex_data_[0].y();
            float max_z = vertex_data_[0].z();

            for (auto &vertex : vertex_data_)
            {
                min_x = std::min(min_x, vertex.x());
                min_y = std::min(min_y, vertex.y());
                min_z = std::min(min_z, vertex.z());

                max_x = std::max(max_x, vertex.x());
                max_y = std::max(max_y, vertex.y());
                max_z = std::max(max_z, vertex.z());
            }
            min = Eigen::Vector3f(min_x, min_y, min_z);
            max = Eigen::Vector3f(max_x, max_y, max_z);
        }
        Eigen::Vector3f center = (max - min) / 2.f + min;

        for(auto& vertex : vertex_data_)
        {
            vertex = vertex - center;
        }

        /* Init aabb */
        min = min - center;
        max = max - center;
        box_ = Eigen::AlignedBox3d(min.cast<double>(), max.cast<double>());

        /* Init Vertex buffer */
        {
            auto &core = Core::get_instance();

            std::string memory_name = fmt::format("MeshVertex{}", push_constant_.instance_id);
            core.mm.add_memory(memory_name,
                               sizeof(Eigen::Vector3f) * vertex_data_.size(),
                               Memory::TYPE::VERTEX, true);
            core.mm.upload_memory(memory_name, vertex_data_.data(), sizeof(Eigen::Vector3f) * vertex_data_.size());
        }

        /* Init Normal buffer */
        {
            auto &core = Core::get_instance();

            std::string memory_name = fmt::format("MeshNormal{}", push_constant_.instance_id);
            core.mm.add_memory(memory_name,
                               sizeof(Eigen::Vector3f) * normal_data_.size(),
                               Memory::TYPE::VERTEX, true);
            core.mm.upload_memory(memory_name, normal_data_.data(), sizeof(Eigen::Vector3f) * normal_data_.size());
        }

        /* Init Index buffer */
        {
            auto &core = Core::get_instance();

            std::string memory_name = fmt::format("MeshIndex{}", push_constant_.instance_id);
            core.mm.add_memory(memory_name,
                               sizeof(uint32_t) * indices_.size(),
                               Memory::TYPE::INDEX, true);
            core.mm.upload_memory(memory_name, indices_.data(), sizeof(uint32_t) * indices_.size());
        }

        /* パイプライン生成 */
        rebuild();
    }

    void Mesh::destroy()
    {
    }

    void Mesh::update(vk::raii::CommandBuffer &command)
    {
        push_constant_.model = get_transform().matrix().cast<float>();

        auto &core = Core::get_instance();

        command.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline_);
        auto vertex_buffer = core.mm.get_memory(fmt::format("MeshVertex{}", push_constant_.instance_id));
        auto normal_buffer = core.mm.get_memory(fmt::format("MeshNormal{}", push_constant_.instance_id));
        auto index_buffer = core.mm.get_memory(fmt::format("MeshIndex{}", push_constant_.instance_id));
        command.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipeline_layout_, 0, {*core.gpu.descriptor_set}, nullptr);
        command.bindVertexBuffers(0, {vertex_buffer.buffer, normal_buffer.buffer}, {0, 0});
        command.pushConstants<PushConstant>(*pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0, push_constant_);

        command.bindIndexBuffer(index_buffer.buffer, 0, vk::IndexType::eUint32);
        command.drawIndexed(static_cast<uint32_t>(indices_.size()), 1, 0, 0, 0);
    }

    void Mesh::rebuild()
    {
        auto &core = Core::get_instance();
        auto extent = core.off_screen.extent;
        auto &shader = core.shader;

        /* Init pipeline */
        std::array<vk::PipelineShaderStageCreateInfo, 2> shader_stages;
        /* Vertexシェーダ */
        {
            shader_stages[0].setStage(vk::ShaderStageFlagBits::eVertex).setPName("main").setModule(shader.get("MESH.VERT"));
        }

        /* Fragmentシェーダ */
        {
            shader_stages[1].setStage(vk::ShaderStageFlagBits::eFragment).setPName("main").setModule(shader.get("MESH.FRAG"));
        }
        std::array<vk::VertexInputBindingDescription, 2> binding_description;
        binding_description[0].binding = 0;
        binding_description[0].setBinding(0).setStride(sizeof(Eigen::Vector3f)).setInputRate(vk::VertexInputRate::eVertex);
        binding_description[1].binding = 1;
        binding_description[1].setBinding(1).setStride(sizeof(Eigen::Vector3f)).setInputRate(vk::VertexInputRate::eVertex);

        std::array<vk::VertexInputAttributeDescription, 2> attribute_description;
        attribute_description[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);
        attribute_description[1].setBinding(1).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat);

        vk::PipelineVertexInputStateCreateInfo vertex_input_state;
        vertex_input_state.setVertexBindingDescriptions(binding_description)
            .setVertexAttributeDescriptions(attribute_description);

        vk::PipelineInputAssemblyStateCreateInfo input_assembly;
        input_assembly.setTopology(vk::PrimitiveTopology::eTriangleList)
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
        colorBlendAttachment[1].setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA).setBlendEnable(vk::False);

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

    int32_t Mesh::get_type_id()
    {
        auto &id = typeid(Mesh);
        return static_cast<int32_t>(id.hash_code());
    }

    int32_t Mesh::get_instance_id()
    {
        return push_constant_.instance_id;
    }

    double Mesh::pick(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction)
    {
        auto& three_d = Core::get_instance().three_d;
        auto pick_data = three_d.get_pick_data();
        auto type_id = get_type_id();
        auto instance_id = get_instance_id();

        if(pick_data.type != get_type_id() || pick_data.instance != get_instance_id())
        {
            return -1.0;
        }

        auto index = indices_[pick_data.vertex];
        auto vertex = vertex_data_[index].cast<double>();

        return (vertex - origin).norm(); 
    }
}
