cmake_minimum_required(VERSION 3.10)
project(GameEngine C)

set(GLFW_DIR "${CMAKE_SOURCE_DIR}/external/glfw")
set(VULKAN_DIR "${CMAKE_SOURCE_DIR}/external/vulkan")

include_directories("${GLFW_DIR}/include")
include_directories("${VULKAN_DIR}/include")

find_library(GLFW_LIBRARY NAMES glfw3 glfw PATHS "${GLFW_DIR}/build/src/Release" NO_DEFAULT_PATH)
find_library(VULKAN_LIBRARY NAMES vulkan-1 vulkan PATHS "${VULKAN_DIR}/Lib" NO_DEFAULT_PATH)

if(NOT GLFW_LIBRARY)
    message(FATAL_ERROR "GLFW library not found in ${GLFW_DIR}/build/src/Release")
endif()
if(NOT VULKAN_LIBRARY)
    message(FATAL_ERROR "Vulkan library not found in ${VULKAN_DIR}/Lib")
endif()

add_library(engine SHARED
        src/engine.c
        src/swapchain.c
        src/renderpass.c
        src/framebuffer.c
        src/command.c
        src/sync.c
        src/buffers.c
        src/pipeline.c
        src/descriptors.c
)

target_link_libraries(engine PRIVATE ${VULKAN_LIBRARY} ${GLFW_LIBRARY})
set_target_properties(engine PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/compiled"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/compiled"
)

if(MSVC)
    target_link_options(engine PRIVATE /NODEFAULTLIB:MSVCRTD)
endif()
