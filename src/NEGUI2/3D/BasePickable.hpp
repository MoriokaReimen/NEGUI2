#ifndef _IPICKABLE_HPP
#define _IPICKABLE_HPP
#include <Eigen/Dense>
namespace NEGUI2
{
    class BasePickable
    {
    protected:
        Eigen::AlignedBox3d box_;
        bool display_aabb_;

    public:
        BasePickable();
        virtual ~BasePickable();
        virtual double pick(const Eigen::Vector3d &origin, const Eigen::Vector3d &direction) = 0;
        bool display_aabb() const;
        void set_display_aabb(const bool aabb = true);
        void toggle_display_aabb();
        Eigen::AlignedBox3d box() const;
    };
}

#endif