file(GLOB src
    "./*.h"
    "./*.cpp"
)

add_library(imgui STATIC "${src}")

target_include_directories(imgui INTERFACE "./")