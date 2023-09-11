#include "NEGUI2/3D/Camera.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/MemoryManager.hpp"
#include <cstdlib>
namespace {
    constexpr double PI = 3.14159265359;

    double to_rad(const double& degree)
    {
        return degree / 180.0 * PI;
    }

    double to_deg(const double& rad)
    {
        return rad / PI * 180.0;
    }

    Eigen::Matrix4d perspective(const double& fovy_degree, const double& aspect, const double& znear, const double& zfar)
    {
        assert(fovy_degree > 1.0);
        double focal = 1.0 / std::tan(to_rad(fovy_degree) / 2.0);
        assert(aspect > 1E-5 || aspect < -1E-5);
        double x = focal / aspect;
        double y = -focal;
        assert((zfar - znear) > 1E-5);
        double alpha = znear / (zfar - znear);
        double beta = zfar * alpha;

        Eigen::Matrix4d projection;
        projection << x,   0.0,   0.0, 0.0,
                      0.0,   y,   0.0, 0.0,
                      0.0, 0.0, alpha, beta,
                      0.0, 0.0,  -1.0, 0.0;
        
        return projection;
    }


}

namespace NEGUI2
{

    Camera::Camera(const double& fovy, const double& aspect, const double& znear, const double& zfar)
    : projection_(Eigen::Matrix4d::Identity()), fovy_(fovy), aspect_(aspect), znear_(znear), zfar_(zfar),
      width_(0.f), height_(0.f), mouse_x_(0.f), mouse_y_(0.f),
      BaseTransform::BaseTransform()
    {
        projection_ = ::perspective(fovy_, aspect_, znear_, zfar_);
        auto& core = Core::get_instance();
        auto& mm = core.mm;
        mm.add_memory("camera", 16 * sizeof(float), Memory::TYPE::UNIFORM);
        mm.add_memory("mouse", 2 * sizeof(float), Memory::TYPE::UNIFORM);
    }

    Camera::~Camera()
    {
    }

    void Camera::set_extent(const uint32_t& width, const uint32_t& height)
    {
        assert(height != 0);
        aspect_ = static_cast<double>(width) / static_cast<double>(height);
        width_ = static_cast<float>(width);
        height_ = static_cast<float>(height);
        projection_ = ::perspective(fovy_, aspect_, znear_, zfar_);
    }

    void Camera::set_mouse(const uint32_t& x, const uint32_t& y)
    {
        mouse_x_ = static_cast<float>(x);
        mouse_y_ = static_cast<float>(y);


    }

    void Camera::upload()
    {
        auto &core = Core::get_instance();
        auto &mm = core.mm;
        
        {
            auto memory = mm.get_memory("camera");
            auto transform = projection_ * transform_.inverse();
            Eigen::Matrix4f pv = transform.matrix().cast<float>();
            std::memcpy(memory.alloc_info.pMappedData, pv.data(), sizeof(float) * 16);
        }

        {
            std::array<float, 4> buffer{width_, height_, mouse_x_, mouse_y_};
            auto memory = mm.get_memory("mouse");
            std::memcpy(memory.alloc_info.pMappedData, buffer.data(), sizeof(float) * buffer.size());
        }
    }
}