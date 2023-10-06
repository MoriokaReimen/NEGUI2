#ifndef _MESH_HPP
#define _MESH_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>
#include <vector>
#include <filesystem>

namespace NEGUI2
{
    class Mesh : public BaseDisplayObject, public BaseTransform
    {
        static uint32_t instance_count_;
        PushConstant push_constant_;
        uint32_t instance_id_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;


        std::vector<Eigen::Vector3f> vertex_data_;
        std::vector<Eigen::Vector3f> normal_data_;
        std::vector<uint32_t> indices_;
        std::vector<Eigen::Vector4f> color_data_;

        public:
        Mesh();
        ~Mesh() override;

        void load(const std::filesystem::path& path);
        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        uint32_t get_type_id() override;
        uint32_t get_instance_id() override;
    };

}
#endif