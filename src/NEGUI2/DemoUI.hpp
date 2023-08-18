#ifndef _DEMO_UI_HPP
#define _DEMO_UI_HPP
#include "NEGUI2/IUserInterface.hpp"

namespace NEGUI2
{
    class DemoUI : public IUserInterface
    {

    public:
        DemoUI();
        virtual ~DemoUI() override;
        virtual void update() override;
    };

}
#endif