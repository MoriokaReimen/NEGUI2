#include "NEGUI2/3D/Cube.hpp"
#include <cstddef>
#include "NEGUI2/Core/Core.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <NEGUI2/Core/Utility.hpp>

struct Vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0};
namespace NEGUI2
{
    Cube::Cube()
    {
    }

    Cube::~Cube()
    {
    }

    void Cube::init(DeviceData &device_data)
    {
        /* デバイスをコピー */
        device_ = device_data.device;
        allocator_ = device_data.allocator;

        /* 頂点バッファ生成 */
        {
            /* バッファ生成 */
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = sizeof(vertices[0]) * vertices.size();
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            auto buffer_result = vmaCreateBuffer(device_data.allocator, &bufferInfo, &allocInfo, &vertex_buffer_, &vertex_allocation_, nullptr);
            check_vk_result(buffer_result);

            /* ステージングバッファ生成 */
            VkBuffer stage_buffer;
            VmaAllocation stage_allocation;
            VmaAllocationInfo alloc_info;
            {
                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = sizeof(vertices[0]) * vertices.size();
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocInfo{};
                allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
                allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                  VMA_ALLOCATION_CREATE_MAPPED_BIT;
                VkBuffer buffer;
                VmaAllocation allocation;
                auto result = vmaCreateBuffer(device_data.allocator, &bufferInfo, &allocInfo, &stage_buffer, &stage_allocation, &alloc_info);
                check_vk_result(result);
            }

            /* データコピー */
            {
                std::memcpy(alloc_info.pMappedData, vertices.data(), sizeof(vertices[0]) * vertices.size());
                auto flush_result = vmaFlushAllocation(device_data.allocator, stage_allocation, 0, VK_WHOLE_SIZE);
                check_vk_result(flush_result);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vkBeginCommandBuffer(device_data.command_buffer, &beginInfo);

                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = sizeof(vertices[0]) * vertices.size();
                vkCmdCopyBuffer(device_data.command_buffer, stage_buffer, vertex_buffer_, 1, &copyRegion);

                vkEndCommandBuffer(device_data.command_buffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &device_data.command_buffer;

                vkQueueSubmit(device_data.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(device_data.graphics_queue);
            }

            /* ステージバッファ削除 */
            vmaDestroyBuffer(device_data.allocator, stage_buffer, stage_allocation);
        }

        /* インデックスバッファ生成 */
        {
            /* バッファ生成 */
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = sizeof(indices[0]) * indices.size();
            bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            vmaCreateBuffer(device_data.allocator, &bufferInfo, &allocInfo, &index_buffer_, &index_allocation_, nullptr);

            /* ステージングバッファ生成 */
            VkBuffer stage_buffer;
            VmaAllocation stage_allocation;
            VmaAllocationInfo alloc_info;
            {
                VkBufferCreateInfo bufferInfo{};
                bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size = sizeof(indices[0]) * indices.size();
                bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
                bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocInfo{};
                allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
                allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                  VMA_ALLOCATION_CREATE_MAPPED_BIT;
                VkBuffer buffer;
                VmaAllocation allocation;
                vmaCreateBuffer(device_data.allocator, &bufferInfo, &allocInfo, &stage_buffer, &stage_allocation, &alloc_info);
            }

            /* データコピー */
            {
                std::memcpy(alloc_info.pMappedData, indices.data(), sizeof(indices[0]) * indices.size());
                vmaFlushAllocation(device_data.allocator, stage_allocation, 0, VK_WHOLE_SIZE);

                VkCommandBufferBeginInfo beginInfo{};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                vkBeginCommandBuffer(device_data.command_buffer, &beginInfo);

                VkBufferCopy copyRegion{};
                copyRegion.srcOffset = 0;
                copyRegion.dstOffset = 0;
                copyRegion.size = sizeof(indices[0]) * indices.size();
                vkCmdCopyBuffer(device_data.command_buffer, stage_buffer, index_buffer_, 1, &copyRegion);

                vkEndCommandBuffer(device_data.command_buffer);

                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &device_data.command_buffer;

                vkQueueSubmit(device_data.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
                vkQueueWaitIdle(device_data.graphics_queue);
            }

            /* ステージバッファ削除 */
            vmaDestroyBuffer(device_data.allocator, stage_buffer, stage_allocation);
        }
    }

    void Cube::create_pipeline(DeviceData &device_data, WindowData &window_data)
    {
        /* パイプライン生成 */
        {
            if(pipeline_ != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device_, pipeline_, nullptr);
                vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
            }

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = device_data.shader.get("base.vert");
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = device_data.shader.get("base.frag");
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            {
                vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


                vertexInputInfo.vertexBindingDescriptionCount = 1;
                vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
                vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
                vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
            }

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float)window_data.width;
            viewport.height = (float)window_data.height;
            viewport.minDepth = 0.0f; 
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = {static_cast<unsigned int>(window_data.width),   static_cast<unsigned int>(window_data.height)};

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.rasterizerDiscardEnable = VK_FALSE;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f;
            colorBlending.blendConstants[1] = 0.0f;
            colorBlending.blendConstants[2] = 0.0f;
            colorBlending.blendConstants[3] = 0.0f;

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;

            if (vkCreatePipelineLayout(device_data.device, &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = pipeline_layout_;
            pipelineInfo.renderPass = window_data.render_pass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(device_data.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create graphics pipeline!");
            }
        }
    }

    void Cube::destroy()
    {
        vmaDestroyBuffer(allocator_, vertex_buffer_, vertex_allocation_);
        vmaDestroyBuffer(allocator_, index_buffer_, index_allocation_);
        vkDestroyPipeline(device_, pipeline_, nullptr);
        vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
    }

    void Cube::update(VkCommandBuffer &command)
    {
        vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);

        VkBuffer vertexBuffers[] = {vertex_buffer_};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(command, index_buffer_, 0, VK_INDEX_TYPE_UINT16);
        // vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 0, nullptr, 0, nullptr);
        vkCmdDrawIndexed(command, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }
}
