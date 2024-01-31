cmake_minimum_required(VERSION 3.24)

project(Peak)

SET(CMAKE_CXX_STANDARD 20)

add_definitions(-DROOT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

set(PEAK_SOURCE_FILES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(PEAK_SHADER_FILES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

file(GLOB_RECURSE PEAK_HEAD_FILES
    ${PEAK_SOURCE_FILES_PATH}/*.h
    ${PEAK_SOURCE_FILES_PATH}/*.hpp
    ${PEAK_SOURCE_FILES_PATH}/*.inl
)
file(GLOB_RECURSE PEAK_SOURCE_FILES
    ${PEAK_SOURCE_FILES_PATH}/*.h
    ${PEAK_SOURCE_FILES_PATH}/*.hpp
    ${PEAK_SOURCE_FILES_PATH}/*.inl
    ${PEAK_SOURCE_FILES_PATH}/*.cpp
    ${PEAK_SOURCE_FILES_PATH}/*.imp
)
file(GLOB_RECURSE PEAK_SHADERS_FILES
    ${PEAK_SHADER_FILES_PATH}/*.vert
    ${PEAK_SHADER_FILES_PATH}/*.frag
)

source_group(TREE ${PEAK_SOURCE_FILES_PATH} FILES ${PEAK_HEAD_FILES})
source_group(TREE ${PEAK_SOURCE_FILES_PATH} FILES ${PEAK_SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PEAK_SHADERS_FILES})

add_executable(Peak
    ${PEAK_HEAD_FILES}
    ${PEAK_SOURCE_FILES}
    ${PEAK_SHADERS_FILES}
)
set_property(TARGET Peak PROPERTY COMPILE_WARNING_AS_ERROR ON)

option(VKB_BUILD_SAMPLES "" OFF)
add_subdirectory(ThirdParty/Vulkan-Samples)

target_include_directories(Peak
    PUBLIC
        ${PEAK_SOURCE_FILES_PATH}
        ThirdParty/Vulkan-Samples/framework
        ThirdParty/Vulkan-Samples/app
)

target_link_libraries(Peak
    PRIVATE
        framework
        apps
        plugins
)
