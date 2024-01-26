cmake_minimum_required(VERSION 3.20)

project(Peak)

SET(CMAKE_CXX_STANDARD 20)

add_executable(Peak src/main.cpp)

find_package(Vulkan REQUIRED)

add_subdirectory(ThirdParty/Vulkan-HPP/glfw)
add_subdirectory(ThirdParty/Vulkan-HPP/glm)
add_subdirectory(ThirdParty/Vulkan-HPP/glslang)

# Vulkan-HPP utils
add_subdirectory(ThirdParty/Vulkan-HPP)
add_subdirectory(ThirdParty/Vulkan-HPP/samples/utils)
add_subdirectory(ThirdParty/Vulkan-HPP/RAII_samples/utils)

target_include_directories(Peak
    PUBLIC
        ThirdParty/Vulkan-HPP/glfw/include
        ThirdParty/Vulkan-HPP/RAII_samples/utils
)
target_link_libraries(Peak PRIVATE
    Vulkan::Vulkan
    glfw
    RAII_utils
)
