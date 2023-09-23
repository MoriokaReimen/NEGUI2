#ifndef _IPICKABLE_HPP
#define _IPICKABLE_HPP

namespace NEGUI2
{
    class IPickable
    {
        public:
        virtual ~IPickable() {}
        virtual double pick(const Eigen::Vector3d& origin, const Eigen::Vector3d& direction) = 0;
    };
}

#endif