cmake_minimum_required(VERSION 3.24)

project(Loom)

SET(CMAKE_CXX_STANDARD 20)

add_definitions(-DROOT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(LOOM_SOURCE_FILES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LOOM_SHADER_FILES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

file(GLOB_RECURSE LOOM_HEAD_FILES
    ${LOOM_SOURCE_FILES_PATH}/*.h
    ${LOOM_SOURCE_FILES_PATH}/*.hpp
    ${LOOM_SOURCE_FILES_PATH}/*.inl
)
file(GLOB_RECURSE LOOM_SOURCE_FILES
    ${LOOM_SOURCE_FILES_PATH}/*.h
    ${LOOM_SOURCE_FILES_PATH}/*.hpp
    ${LOOM_SOURCE_FILES_PATH}/*.inl
    ${LOOM_SOURCE_FILES_PATH}/*.cpp
    ${LOOM_SOURCE_FILES_PATH}/*.imp
)
file(GLOB_RECURSE LOOM_SHADERS_FILES
    ${LOOM_SHADER_FILES_PATH}/*.vert
    ${LOOM_SHADER_FILES_PATH}/*.frag
)

source_group(TREE ${LOOM_SOURCE_FILES_PATH} FILES ${LOOM_HEAD_FILES})
source_group(TREE ${LOOM_SOURCE_FILES_PATH} FILES ${LOOM_SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LOOM_SHADERS_FILES})

add_executable(Loom
    ${LOOM_HEAD_FILES}
    ${LOOM_SOURCE_FILES}
    ${LOOM_SHADERS_FILES}
)
set_property(TARGET Loom PROPERTY COMPILE_WARNING_AS_ERROR ON)

option(VKB_BUILD_SAMPLES "" OFF)
add_subdirectory(ThirdParty/Vulkan-Samples)

target_include_directories(Loom
    PUBLIC
        ${LOOM_SOURCE_FILES_PATH}
        ThirdParty/Vulkan-Samples/framework
        ThirdParty/Vulkan-Samples/app
)

target_link_libraries(Loom
    PRIVATE
        framework
        apps
        plugins
)
