#ifndef _SCENE_HPP
#define _SCENE_HPP
#include "IModule.hpp"
#include "NEGUI2/ThreeD/Camera.hpp"
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
#include <Eigen/Dense>
#include "NEGUI2/ThreeD/BaseDisplayObject.hpp"
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
        std::shared_ptr<NEGUI2::BaseDisplayObject> target_;
    public:
        void handle_camera_();
        public:
        Scene(std::shared_ptr<entt::registry> registry);
        ~Scene() override;
        void init() override;
        void update() override;
    };

}
#endif