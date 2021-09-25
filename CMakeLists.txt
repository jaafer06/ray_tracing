cmake_minimum_required(VERSION 3.6)

project(rayCasting)
set(CMAKE_CXX_STANDARD 14)

file(GLOB source_files
        "src/*.h"
        "src/*.cpp"
)

file(GLOB source_files "src/*.cpp")
file(GLOB imgui_cpp "external/imgui/*.cpp")

add_executable(
        ${PROJECT_NAME}
        ${source_files}
        external/glfw/deps/glad_gl.c
        ${imgui_cpp}
        )
add_subdirectory(external/glfw)
target_include_directories(${PROJECT_NAME} PUBLIC 
        external/glfw/include
        external/glfw/deps
        external/glm/glm
        external/Eigen
        external/stb_image
        external/imgui
        src/
        )
target_link_directories( ${PROJECT_NAME} PRIVATE external/glfw/src)
target_link_libraries(${PROJECT_NAME} glfw)
