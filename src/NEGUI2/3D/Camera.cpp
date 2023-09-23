#include "NEGUI2/3D/Camera.hpp"
#include "NEGUI2/Core/Core.hpp"
#include "NEGUI2/Core/MemoryManager.hpp"
#include <cstdlib>
namespace
{
    constexpr double PI = 3.14159265359;

    double to_rad(const double &degree)
    {
        return degree / 180.0 * PI;
    }

    double to_deg(const double &rad)
    {
        return rad / PI * 180.0;
    }

    Eigen::Matrix4d perspective(const double &fovy_degree, const double &aspect, const double &znear, const double &zfar)
    {
        assert(fovy_degree > 1.0);
        double focal = 1.0 / std::tan(to_rad(fovy_degree) / 2.0);
        assert(aspect > 1E-5 || aspect < -1E-5);
        double x = focal / aspect;
        double y = focal;
        assert((zfar - znear) > 1E-5);
        double alpha = zfar / (zfar - znear);
        double beta = -znear * alpha;

        Eigen::Matrix4d projection;
        projection << 
              x, 0.0,   0.0, 0.0,
            0.0,   y,   0.0, 0.0,
            0.0, 0.0, alpha, beta,
            0.0, 0.0,   1.0, 0.0;

        return projection;
    }

    uint32_t timeSinceEpochMillisec()
    {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }

    struct CameraData
    {
        Eigen::Matrix4f transform;
        Eigen::Vector2f resolution;
        uint32_t time_ms;
    };
}

namespace NEGUI2
{

    Camera::Camera(const double &fovy, const double &aspect, const double &znear, const double &zfar)
        : projection_(Eigen::Matrix4d::Identity()), fovy_(fovy), aspect_(aspect), znear_(znear), zfar_(zfar),
          width_(1920.f), height_(1080.f), mouse_x_(0.f), mouse_y_(0.f),
          BaseTransform::BaseTransform()
    {
        aspect_ = static_cast<double>(width_) / static_cast<double>(height_);
        projection_ = ::perspective(fovy_, aspect_, znear_, zfar_);
        auto &core = Core::get_instance();

        auto &mm = core.mm;
        {
            mm.add_memory("camera", sizeof(CameraData), Memory::TYPE::UNIFORM);
            mm.add_memory("mouse", 2 * sizeof(float), Memory::TYPE::UNIFORM);
        }

        auto &gpu = core.gpu;
        {
            std::array<vk::DescriptorBufferInfo, 2> buffer_infos;
            auto mouse_memory = mm.get_memory("mouse");
            buffer_infos[0].setBuffer(mouse_memory.buffer).setOffset(0u).setRange(vk::WholeSize);

            auto camera_memory = mm.get_memory("camera");
            buffer_infos[1].setBuffer(camera_memory.buffer).setOffset(0u).setRange(vk::WholeSize);

            std::array<vk::WriteDescriptorSet, 2> write_descriptor_sets;
            write_descriptor_sets[0].setDstSet(*gpu.descriptor_set).setDstBinding(0u).setDstArrayElement(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setBufferInfo(buffer_infos[0]);
            write_descriptor_sets[1].setDstSet(*gpu.descriptor_set).setDstBinding(1u).setDstArrayElement(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setBufferInfo(buffer_infos[1]);

            gpu.device.updateDescriptorSets(write_descriptor_sets, nullptr);
        }
    }

    Camera::~Camera()
    {
    }

    void Camera::set_extent(const uint32_t &width, const uint32_t &height)
    {
        assert(height != 0);
        aspect_ = static_cast<double>(width) / static_cast<double>(height);
        width_ = static_cast<float>(width);
        height_ = static_cast<float>(height);
        projection_ = ::perspective(fovy_, aspect_, znear_, zfar_);
    }

    void Camera::set_extent(const vk::Extent2D &extent)
    {
        set_extent(extent.width, extent.height);
    }

    void Camera::set_mouse(const uint32_t &x, const uint32_t &y)
    {
        mouse_x_ = static_cast<float>(x);
        mouse_y_ = static_cast<float>(y);
    }

    void Camera::upload()
    {
        auto &core = Core::get_instance();
        auto &mm = core.mm;

        {
            CameraData camera_data;
            auto memory = mm.get_memory("camera");
            auto transform = projection_ * transform_.matrix().inverse();
            camera_data.transform = transform.matrix().cast<float>();
            camera_data.resolution = Eigen::Vector2f(width_, height_);
            camera_data.time_ms = ::timeSinceEpochMillisec();

            std::memcpy(memory.alloc_info.pMappedData, &camera_data, sizeof(CameraData));
        }

        {
            std::array<float, 4> buffer{width_, height_, mouse_x_, mouse_y_};
            auto memory = mm.get_memory("mouse");
            std::memcpy(memory.alloc_info.pMappedData, buffer.data(), sizeof(float) * buffer.size());
        }
    }

    void Camera::lookat(const Eigen::Vector3d &target, const Eigen::Vector3d &up)
    {
        auto translation = get_transform() * Eigen::Vector4d::UnitW();

        Eigen::Vector3d front = target - translation.head<3>();
        auto right = front.cross(up);
        if (right.squaredNorm() < 1E-4)
        {
            right = Eigen::Vector3d::UnitX();
        }
        right = right;
        auto cross = right.cross(right);
        auto quat = Eigen::Quaterniond::FromTwoVectors(Eigen::Vector3d::UnitZ(), front);
        quat = Eigen::Quaterniond::FromTwoVectors(quat * Eigen::Vector3d::UnitX(), right) * quat;

        set_orientation(quat.matrix());
    }

    Eigen::Vector3d Camera::uv_to_near_xyz(const Eigen::Vector2d &uv) const
    {
        auto vec = Eigen::Vector4d(uv.x(), uv.y(), 0.0, 0.0);
        auto ndc = projection_.inverse() * vec;
        auto ret = ndc.head<3>() / ndc.w();
        return ret;
    }

    Eigen::Vector3d Camera::uv_to_far_xyz(const Eigen::Vector2d &uv) const
    {
        auto vec = Eigen::Vector4d(uv.x(), uv.y(), 1.0, 0.0);
        auto ndc = projection_.inverse() * vec;
        auto ret = ndc.head<3>() / ndc.w();

        return ret;
    }

    Eigen::Vector3d Camera::uv_to_direction(const Eigen::Vector2d &uv) const
    {
        auto near = uv_to_near_xyz(uv);
        auto far = uv_to_far_xyz(uv);
        auto ret = far - near;
        return ret.normalized();
    }
}