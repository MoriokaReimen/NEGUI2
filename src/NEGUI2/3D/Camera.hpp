#ifndef _CAMERA_HPP
#define _CAMERA_HPP
#include <Eigen/Dense>
#include <vulkan/vulkan_raii.hpp>
#include "NEGUI2/3D/BaseTransform.hpp"
#include <cinttypes>

namespace NEGUI2
{
    class Camera : BaseTransform
    {
        Eigen::Matrix4d projection_;
        double fovy_;
        double aspect_;
        double znear_;
        double zfar_;

        float width_;
        float height_;
        float mouse_x_;
        float mouse_y_;

    public:
        Camera(const double& fovy, const double& aspect = 1.0, const double& znear = 0.1, const double& zfar = 500.0);
        ~Camera() override;
        void set_extent(const uint32_t& width, const uint32_t& height);
        void set_mouse(const uint32_t& x, const uint32_t& y);
        void upload();

    };
}

#endif