#include "NEGUI2/Ui/PlotDemo.hpp"
#include <implot.h>

namespace NEGUI2
{
    PlotDemo::PlotDemo()
        : IUserInterface::IUserInterface()
    {
    }

    PlotDemo::~PlotDemo()
    {
    }

    void PlotDemo::update()
    {
        if (is_active_)
        {
            ImPlot::ShowDemoWindow(&is_active_);
        }
    }
}