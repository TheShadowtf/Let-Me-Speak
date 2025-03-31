cmake_minimum_required(VERSION 3.10)
project(LMS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# ✅ Find OpenGL
find_package(OpenGL REQUIRED)

# ✅ Include Source Files
file(GLOB_RECURSE SRC_FILES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ✅ Include Headers
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/glfw/include
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/imgui/backends
)

# ✅ Create Executable
add_executable(LMS ${SRC_FILES})

# ✅ Add Glad
add_library(glad "vendor/glad/src/glad.c")
target_include_directories(glad PUBLIC "vendor/glad/include")
target_link_libraries(LMS PRIVATE glad)

# ✅ Add GLFW
add_subdirectory(vendor/glfw)
target_link_libraries(LMS PRIVATE glfw OpenGL::GL)

# ✅ Add ImGui
add_library(imgui
    vendor/imgui/imgui.cpp
    vendor/imgui/imgui_draw.cpp
    vendor/imgui/imgui_widgets.cpp
    vendor/imgui/imgui_tables.cpp
    vendor/imgui/backends/imgui_impl_glfw.cpp
    vendor/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui vendor/imgui/backends)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL glad)
target_link_libraries(LMS PRIVATE imgui)
