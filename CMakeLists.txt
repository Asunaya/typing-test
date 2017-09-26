cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include("ucm")

add_subdirectory("lib/gl3w/")
add_subdirectory("lib/imgui/")
add_subdirectory("lib/imgui_impl_glfw_gl3/")
add_subdirectory("typing-test/")

ucm_set_runtime(STATIC)