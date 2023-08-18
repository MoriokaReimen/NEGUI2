include_guard(GLOBAL)

include(FetchContent)

##################################################
# google test
##################################################
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.13.0
)
FetchContent_MakeAvailable(googletest)

##################################################
# spdlog
##################################################
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0
)
FetchContent_MakeAvailable(spdlog)

##################################################
# vulkan headers
##################################################
FetchContent_Declare(
  vulkan_headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG        v1.3.261
)
FetchContent_MakeAvailable(vulkan_headers)

##################################################
# glfw
##################################################
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

##################################################
# glm
##################################################
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8

)
FetchContent_MakeAvailable(glm)

##################################################
# glslang
##################################################
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG        sdk-1.3.250.0
)
FetchContent_MakeAvailable(glslang)

##################################################
# vma
##################################################
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG        v3.0.1
)
FetchContent_MakeAvailable(vma)

##################################################
# imgui
##################################################
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
  )
  FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
  find_package(glfw3 REQUIRED)
  find_package(Vulkan REQUIRED)
  FetchContent_Populate(imgui)

    add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  )
  target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)
  add_library(imgui::imgui ALIAS imgui)
  target_include_directories(imgui
  PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
endif ()

##################################################
# implot
##################################################
FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG v0.15
  )
  FetchContent_GetProperties(implot)
if (NOT implot_POPULATED)
  FetchContent_Populate(implot)
  add_library(implot STATIC
  ${implot_SOURCE_DIR}/implot.cpp
  ${implot_SOURCE_DIR}/implot_demo.cpp
  ${implot_SOURCE_DIR}/implot_items.cpp
  )
  target_link_libraries(implot PUBLIC imgui::imgui)
  add_library(imgui::implot ALIAS implot)
  target_include_directories(implot
  PUBLIC
    ${implot_SOURCE_DIR}
  )
endif ()
