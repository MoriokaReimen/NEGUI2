// This file is part of Eigen, a lightweight C++ template library
#ifndef _CAMERA_HPP
#define _CAMERA_HPP
#include <Eigen/Geometry>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace NEGUI2
{
  class DeviceData;
  class WindowData;

  class Frame
  {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    inline Frame(const Eigen::Vector3f &pos = Eigen::Vector3f::Zero(),
                 const Eigen::Quaternionf &o = Eigen::Quaternionf())
        : orientation(o), position(pos)
    {
    }
    Frame lerp(float alpha, const Frame &other) const
    {
      return Frame((1.f - alpha) * position + alpha * other.position,
                   orientation.slerp(alpha, other.orientation));
    }

    Eigen::Quaternionf orientation;
    Eigen::Vector3f position;
  };

  class Camera
  {
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Camera(void);

    Camera(const Camera &other) = delete;            /* コピー禁止 */
    Camera &operator=(const Camera &other) = delete; /* コピー禁止 */

    virtual ~Camera();

    void init(DeviceData &device_data);
    void destroy();
    void upload(WindowData &window_data);
    VkBuffer get_uniform() const;

    void setViewport(uint offsetx, uint offsety, uint width, uint height);
    void setViewport(uint width, uint height);

    inline uint vpX(void) const { return mVpX; }
    inline uint vpY(void) const { return mVpY; }
    inline uint vpWidth(void) const { return mVpWidth; }
    inline uint vpHeight(void) const { return mVpHeight; }

    inline float fovY(void) const { return mFovY; }
    void setFovY(float value);

    void setPosition(const Eigen::Vector3f &pos);
    inline const Eigen::Vector3f &position(void) const { return mFrame.position; }

    void setOrientation(const Eigen::Quaternionf &q);
    inline const Eigen::Quaternionf &orientation(void) const { return mFrame.orientation; }

    void setFrame(const Frame &f);
    const Frame &frame(void) const { return mFrame; }

    void setDirection(const Eigen::Vector3f &newDirection);
    Eigen::Vector3f direction(void) const;
    void setUp(const Eigen::Vector3f &vectorUp);
    Eigen::Vector3f up(void) const;
    Eigen::Vector3f right(void) const;

    void setTarget(const Eigen::Vector3f &target);
    inline const Eigen::Vector3f &target(void) { return mTarget; }

    const Eigen::Affine3f &viewMatrix(void) const;
    const Eigen::Matrix4f &projectionMatrix(void) const;

    void rotateAroundTarget(const Eigen::Quaternionf &q);
    void localRotate(const Eigen::Quaternionf &q);
    void zoom(float d);

    void localTranslate(const Eigen::Vector3f &t);

    Eigen::Vector3f unProject(const Eigen::Vector2f &uv, float depth, const Eigen::Matrix4f &invModelview) const;
    Eigen::Vector3f unProject(const Eigen::Vector2f &uv, float depth) const;

  protected:
    void updateViewMatrix(void) const;
    void updateProjectionMatrix(void) const;

  protected:
    VmaAllocator allocator_;
    VkBuffer uniform_buffer_;
    VmaAllocation allocation_;
    VmaAllocationInfo alloc_info_;

    uint mVpX, mVpY;
    uint mVpWidth, mVpHeight;

    Frame mFrame;

    mutable Eigen::Affine3f mViewMatrix;
    mutable Eigen::Matrix4f mProjectionMatrix;

    mutable bool mViewIsUptodate;
    mutable bool mProjIsUptodate;

    // used by rotateAroundTarget
    Eigen::Vector3f mTarget;

    float mFovY;
    float mNearDist;
    float mFarDist;
  };
}
#endif