cmake_minimum_required(VERSION 3.6)

project(rayCasting)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-std=c++17")

file(GLOB source_files
        "src/main.cpp"
)

file(GLOB header_files
        "src/*.h"
        "src/gpu_version/*.h"
        "src/cpu_version/*.h"
)

file(GLOB imgui_cpp "external/imgui/*.cpp")

add_executable(
        ${PROJECT_NAME}
        ${source_files}
        external/glad/src/glad.c
        ${imgui_cpp}
        ${header_files}

        )
add_subdirectory(external/glfw)
target_include_directories(${PROJECT_NAME} PUBLIC 
        external/glfw/include
        external/glad/include
        external/glm/glm
        external/Eigen
        external/stb_image
        external/imgui
        src/
        )

target_link_directories( ${PROJECT_NAME} PRIVATE external/glfw/src)
target_link_libraries(${PROJECT_NAME} glfw)
