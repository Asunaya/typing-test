file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_library(imgui_impl_glfw_gl3 STATIC "${src}")

target_include_directories(imgui_impl_glfw_gl3 INTERFACE "./" PUBLIC "../glfw/include/")

add_dependencies(imgui_impl_glfw_gl3 imgui gl3w)
target_link_libraries(imgui_impl_glfw_gl3 PUBLIC
	imgui
	gl3w
	"${CMAKE_CURRENT_SOURCE_DIR}/../glfw/lib-vc2015/glfw3.lib"
	"Opengl32.lib")