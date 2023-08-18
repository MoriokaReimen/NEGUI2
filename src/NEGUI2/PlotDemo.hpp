#ifndef _PLOT_DEMO_HPP
#define _PLOT_DEMO_HPP
#include "NEGUI2/IUserInterface.hpp"

namespace NEGUI2
{
    class PlotDemo : public IUserInterface
    {

    public:
        PlotDemo();
        virtual ~PlotDemo() override;
        virtual void update() override;
    };

}
#endif