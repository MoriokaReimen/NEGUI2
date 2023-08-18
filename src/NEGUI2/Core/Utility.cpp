#include "NEGUI2/Core/Utility.hpp"
#include <spdlog/spdlog.h>
#include <cstdlib>
namespace NEGUI2
{
    void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        spdlog::error("[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            std::abort();
    }
}
