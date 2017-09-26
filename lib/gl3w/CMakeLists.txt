file(GLOB src
    "src/*.c"
)

add_library(gl3w STATIC "${src}")

target_include_directories(gl3w PUBLIC "./include/")
