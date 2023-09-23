#ifndef _SCENE_HPP
#define _SCENE_HPP
#include "IModule.hpp"
#include "NEGUI2/ThreeD/Camera.hpp"
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include <Eigen/Dense>

namespace App
{
    class Scene : public IModule
    {
    public:
        struct Context {
            Eigen::Vector3d position;
            Eigen::Vector3d direction;
        };
    private:
        Context context_;
    public:
        NEGUI2::Camera camera_;
        void handle_camera_();
        public:
        Scene(std::shared_ptr<entt::registry> registry);
        ~Scene() override;
        void init() override;
        void update() override;
    };

}
#endif