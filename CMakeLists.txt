cmake_minimum_required(VERSION 2.8...3.27)
project(Sample)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

##################################################
# Configure CMake Modules
##################################################
find_package(Vulkan REQUIRED)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/External.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Shader.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/RuntimeResource.cmake)

##################################################
# Configure NEGUI2 Library
##################################################
file(GLOB_RECURSE NEGUI2_SRC ${CMAKE_CURRENT_LIST_DIR}/src/NEGUI2/*.cpp)
add_library(NEGUI2 ${NEGUI2_SRC})
target_include_directories(NEGUI2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(NEGUI2 PUBLIC glfw spdlog::spdlog Vulkan::Vulkan Vulkan::Headers imgui::imgui
                                    imgui::implot imgui::imguizmo imgui::colortextedit Eigen3::Eigen stb::stb
                                    glslang glslang-default-resource-limits SPIRV VulkanMemoryAllocator assimp::assimp)

file(GLOB_RECURSE GLSL_SRC ${CMAKE_CURRENT_LIST_DIR}/shader/*.frag ${CMAKE_CURRENT_LIST_DIR}/shader/*.vert)
target_glsl_shaders(NEGUI2 PUBLIC ${GLSL_SRC})
target_compile_definitions(NEGUI2 PUBLIC VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=0)
file(GLOB NEGUI_RUNTIME ${CMAKE_CURRENT_LIST_DIR}/resource/*.*)
target_runtime_resource(NEGUI2 PUBLIC ${NEGUI_RUNTIME})

##################################################
# Configure App Executable
##################################################
file(GLOB APP_SRC ${CMAKE_CURRENT_LIST_DIR}/src/App/*.cpp)
add_executable(App ${APP_SRC})
target_link_libraries(App PRIVATE NEGUI2 EnTT::EnTT portable_file_dialogs)
target_include_directories(App PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/App)

##################################################
# Configure Test Executable
##################################################
file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
add_executable(Test ${TEST_SRC})
target_link_libraries(Test PRIVATE GTest::gtest_main)
