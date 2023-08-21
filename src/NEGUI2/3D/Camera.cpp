#include "Camera.hpp"
#include <Eigen/LU>

#include "NEGUI2/Core/Utility.hpp"
#include "NEGUI2/Core/Core.hpp"

namespace
{
  struct Uniform
  {
    float projection[16];
    float view[16];
  };
}

namespace NEGUI2
{

  Camera::Camera()
      : mViewIsUptodate(false), mProjIsUptodate(false)
  {
    mViewMatrix.setIdentity();

    mFovY = M_PI / 3.;
    mNearDist = 1.;
    mFarDist = 50000.;

    mVpX = 0;
    mVpY = 0;

    setPosition(Eigen::Vector3f::Constant(100.));
    setTarget(Eigen::Vector3f::Zero());
  }

  Camera::~Camera()
  {
  }

  void Camera::init(DeviceData &device_data)
  {
    allocator_ = device_data.allocator;

    /* バッファ作成 */
    {
      VkBufferCreateInfo bufferInfo{};
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size = sizeof(Uniform);
      bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      VmaAllocationCreateInfo allocInfo{};
      allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
      allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT;
      auto result = vmaCreateBuffer(allocator_, &bufferInfo, &allocInfo, &uniform_buffer_, &allocation_, &alloc_info_);
      check_vk_result(result);
    }
  }

  void Camera::destroy()
  {
    vmaDestroyBuffer(allocator_, uniform_buffer_, allocation_);
  }

  void Camera::upload(WindowData &window_data)
  {
    setViewport(window_data.width, window_data.height);
    updateViewMatrix();
    updateProjectionMatrix();

    auto projection = mProjectionMatrix;
    auto view = mViewMatrix.matrix();
    Uniform uniform;
    Eigen::Map<Eigen::Matrix4f>(uniform.projection, projection.rows(), projection.cols()) = projection;
    Eigen::Map<Eigen::Matrix4f>(uniform.view, view.rows(), view.cols()) = view;
    std::memcpy(alloc_info_.pMappedData, &uniform, sizeof(Uniform));
    vmaFlushAllocation(allocator_, allocation_, 0, VK_WHOLE_SIZE);
  }

  VkBuffer Camera::get_uniform() const
  {
    return uniform_buffer_;
  }

  void Camera::setViewport(uint offsetx, uint offsety, uint width, uint height)
  {
    mVpX = offsetx;
    mVpY = offsety;
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
  }

  void Camera::setViewport(uint width, uint height)
  {
    mVpWidth = width;
    mVpHeight = height;

    mProjIsUptodate = false;
  }

  void Camera::setFovY(float value)
  {
    mFovY = value;
    mProjIsUptodate = false;
  }

  Eigen::Vector3f Camera::direction(void) const
  {
    return -(orientation() * Eigen::Vector3f::UnitZ());
  }

  Eigen::Vector3f Camera::up(void) const
  {
    return orientation() * Eigen::Vector3f::UnitY();
  }

  Eigen::Vector3f Camera::right(void) const
  {
    return orientation() * Eigen::Vector3f::UnitX();
  }

  void Camera::setDirection(const Eigen::Vector3f &newDirection)
  {
    // TODO implement it computing the rotation between newDirection and current dir ?
    Eigen::Vector3f up = this->up();

    Eigen::Matrix3f camAxes;

    camAxes.col(2) = (-newDirection).normalized();
    camAxes.col(0) = up.cross(camAxes.col(2)).normalized();
    camAxes.col(1) = camAxes.col(2).cross(camAxes.col(0)).normalized();
    setOrientation(Eigen::Quaternionf(camAxes));

    mViewIsUptodate = false;
  }

  void Camera::setTarget(const Eigen::Vector3f &target)
  {
    mTarget = target;
    if (!mTarget.isApprox(position()))
    {
      Eigen::Vector3f newDirection = mTarget - position();
      setDirection(newDirection.normalized());
    }
  }

  void Camera::setPosition(const Eigen::Vector3f &p)
  {
    mFrame.position = p;
    mViewIsUptodate = false;
  }

  void Camera::setOrientation(const Eigen::Quaternionf &q)
  {
    mFrame.orientation = q;
    mViewIsUptodate = false;
  }

  void Camera::setFrame(const Frame &f)
  {
    mFrame = f;
    mViewIsUptodate = false;
  }

  void Camera::rotateAroundTarget(const Eigen::Quaternionf &q)
  {
    Eigen::Matrix4f mrot, mt, mtm;

    // update the transform matrix
    updateViewMatrix();
    Eigen::Vector3f t = mViewMatrix * mTarget;

    mViewMatrix = Eigen::Translation3f(t) * q * Eigen::Translation3f(-t) * mViewMatrix;

    Eigen::Quaternionf qa(mViewMatrix.linear());
    qa = qa.conjugate();
    setOrientation(qa);
    setPosition(-(qa * mViewMatrix.translation()));

    mViewIsUptodate = true;
  }

  void Camera::localRotate(const Eigen::Quaternionf &q)
  {
    float dist = (position() - mTarget).norm();
    setOrientation(orientation() * q);
    mTarget = position() + dist * direction();
    mViewIsUptodate = false;
  }

  void Camera::zoom(float d)
  {
    float dist = (position() - mTarget).norm();
    if (dist > d)
    {
      setPosition(position() + direction() * d);
      mViewIsUptodate = false;
    }
  }

  void Camera::localTranslate(const Eigen::Vector3f &t)
  {
    Eigen::Vector3f trans = orientation() * t;
    setPosition(position() + trans);
    setTarget(mTarget + trans);

    mViewIsUptodate = false;
  }

  void Camera::updateViewMatrix(void) const
  {
    if (!mViewIsUptodate)
    {
      Eigen::Quaternionf q = orientation().conjugate();
      mViewMatrix.linear() = q.toRotationMatrix();
      mViewMatrix.translation() = -(mViewMatrix.linear() * position());

      mViewIsUptodate = true;
    }
  }

  const Eigen::Affine3f &Camera::viewMatrix(void) const
  {
    updateViewMatrix();
    return mViewMatrix;
  }

  void Camera::updateProjectionMatrix(void) const
  {
    if (!mProjIsUptodate)
    {
      mProjectionMatrix.setIdentity();
      float aspect = float(mVpWidth) / float(mVpHeight);
      float theta = mFovY * 0.5;
      float range = mFarDist - mNearDist;
      float invtan = 1. / tan(theta);

      mProjectionMatrix(0, 0) = invtan / aspect;
      mProjectionMatrix(1, 1) = invtan;
      mProjectionMatrix(2, 2) = -(mNearDist + mFarDist) / range;
      mProjectionMatrix(3, 2) = -1;
      mProjectionMatrix(2, 3) = -2 * mNearDist * mFarDist / range;
      mProjectionMatrix(3, 3) = 0;

      mProjIsUptodate = true;
    }
  }

  const Eigen::Matrix4f &Camera::projectionMatrix(void) const
  {
    updateProjectionMatrix();
    return mProjectionMatrix;
  }

  Eigen::Vector3f Camera::unProject(const Eigen::Vector2f &uv, float depth) const
  {
    Eigen::Matrix4f inv = mViewMatrix.inverse().matrix();
    return unProject(uv, depth, inv);
  }

  Eigen::Vector3f Camera::unProject(const Eigen::Vector2f &uv, float depth, const Eigen::Matrix4f &invModelview) const
  {
    updateViewMatrix();
    updateProjectionMatrix();

    Eigen::Vector3f a(2. * uv.x() / float(mVpWidth) - 1., 2. * uv.y() / float(mVpHeight) - 1., 1.);
    a.x() *= depth / mProjectionMatrix(0, 0);
    a.y() *= depth / mProjectionMatrix(1, 1);
    a.z() = -depth;
    // FIXME /\/|
    Eigen::Vector4f b = invModelview * Eigen::Vector4f(a.x(), a.y(), a.z(), 1.);
    return Eigen::Vector3f(b.x(), b.y(), b.z());
  }
}