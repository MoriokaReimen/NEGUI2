#ifndef _COORDINATE_HPP
#define _COORDINATE_HPP
#include "NEGUI2/3D/IDisplayObject.hpp"
#include <Eigen/Dense>

namespace NEGUI2
{
    class Coordinate : public IDisplayObject
    {
        static uint32_t instance_count_;
        uint32_t instance_id_;
        vk::raii::Pipeline pipeline_;
        vk::raii::PipelineLayout pipeline_layout_;

        std::array<float, 6 * 3> vertex_data_;

        public:
        Coordinate();
        ~Coordinate() override;

        void init() override;
        void destroy() override;
        void update(vk::raii::CommandBuffer &command) override;
        void rebuild() override;
        uint32_t get_type_id() override;
        uint32_t get_instance_id() override;
    };

}
#endif