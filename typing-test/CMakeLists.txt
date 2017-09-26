file(GLOB src
    "src/*.h"
    "src/*.cpp"
)

add_executable(typing-test "${src}")

add_dependencies(typing-test imgui imgui_impl_glfw_gl3)
target_link_libraries(typing-test PUBLIC imgui imgui_impl_glfw_gl3)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	ucm_add_linker_flags(/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
	target_compile_options(typing-test PUBLIC "/std:c++17")
endif()