cmake_minimum_required(VERSION 3.27)
project(Sample)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


##################################################
# Configure CMake Modules
##################################################
include(${CMAKE_CURRENT_LIST_DIR}/cmake/External.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/Shader.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/RuntimeResource.cmake)

##################################################
# Configure NEGUI2 Library
##################################################
file(GLOB_RECURSE NEGUI2_SRC ${CMAKE_CURRENT_LIST_DIR}/src/NEGUI2/*.cpp)
add_library(NEGUI2 ${NEGUI2_SRC})
target_include_directories(NEGUI2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(NEGUI2 PUBLIC glfw spdlog::spdlog Vulkan::Vulkan Vulkan::Headers  glm::glm imgui::imgui
                                    imgui::implot imgui::imguizmo imgui::colortextedit
                                    glslang glslang-default-resource-limits SPIRV VulkanMemoryAllocator)
target_glsl_shaders(NEGUI2 PUBLIC ${CMAKE_CURRENT_LIST_DIR}/shader/base.frag ${CMAKE_CURRENT_LIST_DIR}/shader/base.vert)
target_compile_definitions(NEGUI2 PUBLIC VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1)
##################################################
# Configure Sample Executable
##################################################
file(GLOB SAMPLE_SRC ${CMAKE_CURRENT_LIST_DIR}/Sample/*.cpp)
add_executable(Sample ${SAMPLE_SRC})
target_link_libraries(Sample PRIVATE Vulkan::Vulkan Vulkan::Headers imgui::imgui)

##################################################
# Configure App Executable
##################################################
file(GLOB APP_SRC ${CMAKE_CURRENT_LIST_DIR}/App/*.cpp)
add_executable(App ${APP_SRC})
target_link_libraries(App PRIVATE NEGUI2)

##################################################
# Configure Test Executable
##################################################
file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp)
add_executable(Test ${TEST_SRC})
target_link_libraries(Test PRIVATE GTest::gtest_main)
