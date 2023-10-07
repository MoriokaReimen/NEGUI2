include_guard(GLOBAL)

include(FetchContent)

# #################################################
# google test
# #################################################
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(googletest)

# #################################################
# spdlog
# #################################################
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# #################################################
# vulkan headers
# #################################################
FetchContent_Declare(
  vulkan_headers
  GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
  GIT_TAG v1.3.261
)
FetchContent_MakeAvailable(vulkan_headers)

# #################################################
# glfw
# #################################################
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

# #################################################
# Eigen
# #################################################
FetchContent_Declare(
  Eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.4.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(EIGEN_BUILD_DOC OFF)
set(EIGEN_BUILD_PKGCONFIG OFF)
FetchContent_MakeAvailable(Eigen)

# #################################################
# glslang
# #################################################
FetchContent_Declare(
  glslang
  GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
  GIT_TAG sdk-1.3.250.0
)
FetchContent_MakeAvailable(glslang)

# #################################################
# vma
# #################################################
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG master
)
FetchContent_MakeAvailable(vma)

# #################################################
# imgui
# #################################################
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG docking
)
FetchContent_GetProperties(imgui)

if(NOT imgui_POPULATED)
  find_package(glfw3 REQUIRED)
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
  target_link_libraries(imgui PUBLIC glfw)
  target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)
  add_library(imgui::imgui ALIAS imgui)
  target_include_directories(imgui
    PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
  )
endif()

# #################################################
# implot
# #################################################
FetchContent_Declare(
  implot
  GIT_REPOSITORY https://github.com/epezent/implot.git
  GIT_TAG v0.15
)
FetchContent_GetProperties(implot)

if(NOT implot_POPULATED)
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
endif()

# #################################################
# imguizmo
# #################################################
FetchContent_Declare(
  imguizmo
  GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
  GIT_TAG master
)
FetchContent_GetProperties(imguizmo)

if(NOT imguizmo_POPULATED)
  FetchContent_Populate(imguizmo)
  add_library(imguizmo STATIC
    ${imguizmo_SOURCE_DIR}/GraphEditor.cpp
    ${imguizmo_SOURCE_DIR}/ImCurveEdit.cpp
    ${imguizmo_SOURCE_DIR}/ImGradient.cpp
    ${imguizmo_SOURCE_DIR}/ImGuizmo.cpp
    ${imguizmo_SOURCE_DIR}/ImSequencer.cpp
  )
  target_link_libraries(imguizmo PUBLIC imgui::imgui)
  add_library(imgui::imguizmo ALIAS imguizmo)
  target_include_directories(imguizmo
    PUBLIC
    ${imguizmo_SOURCE_DIR}
  )
endif()

# #################################################
# colortextedit
# #################################################
FetchContent_Declare(
  colortextedit
  GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit.git
  GIT_TAG master
)
FetchContent_GetProperties(colortextedit)

if(NOT colortextedit_POPULATED)
  FetchContent_Populate(colortextedit)
  add_library(colortextedit STATIC
    ${colortextedit_SOURCE_DIR}/TextEditor.cpp
  )
  target_link_libraries(colortextedit PUBLIC imgui::imgui)
  add_library(imgui::colortextedit ALIAS colortextedit)
  target_include_directories(colortextedit
    PUBLIC
    ${colortextedit_SOURCE_DIR}
  )
endif()

# #################################################
# stb
# #################################################
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_GetProperties(stb)

if(NOT stb_POPULATED)
  FetchContent_Populate(stb)
  add_library(stb INTERFACE)
  add_library(stb::stb ALIAS stb)
  target_include_directories(stb
    INTERFACE
    ${stb_SOURCE_DIR}
  )
endif()

# #################################################
# assimp
# #################################################
FetchContent_Declare(
  assimp
  GIT_REPOSITORY  https://github.com/assimp/assimp.git
  GIT_TAG         v5.3.1
)
FetchContent_MakeAvailable(assimp)


# #################################################
# pfd
# #################################################
FetchContent_Declare(
  pfd
  GIT_REPOSITORY  https://github.com/samhocevar/portable-file-dialogs.git
  GIT_TAG         main
)
FetchContent_MakeAvailable(pfd)

# #################################################
# entt
# #################################################
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG v3.12.2
)
FetchContent_MakeAvailable(entt)
