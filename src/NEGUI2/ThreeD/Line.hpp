#ifndef _LINE_HPP
#define _LINE_HPP
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include "NEGUI2/ThreeD/BaseTransform.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Line : public BaseDisplayObject, BaseTransform
    {
    public:
        struct LineData
        {
            Eigen::Vector3f start;
            Eigen::Vector3f end;
            Eigen::Vector4f color;
            float diameter;
        };

    private:
        static int32_t instance_count_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;
        PushConstant push_constant_;

        std::vector<LineData> line_data_;

    public:
        Line();
        ~Line() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        int32_t get_type_id() override;
        int32_t get_instance_id() override;

        bool add(const Eigen::Vector3f &start, const Eigen::Vector3f &end,
                 const Eigen::Vector4f &color = Eigen::Vector4f::UnitW(), const float &diameter = 0.25);
        bool popback();
        LineData get(size_t index) const;
        size_t size() const;
    };
}
#endif