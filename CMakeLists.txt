cmake_minimum_required(VERSION 3.20)
project(AtmoGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm CONFIG QUIET)
if(NOT TARGET glm::glm)
    find_path(GLM_INCLUDE_DIR glm/glm.hpp REQUIRED)
    add_library(glm::glm INTERFACE IMPORTED)
    set_target_properties(glm::glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}")
endif()

add_executable(AtmoGL
    src/main.cpp
    src/core/Application.cpp
    src/core/Window.cpp
    src/graphics/Mesh.cpp
    src/graphics/MeshFactory.cpp
    src/graphics/Renderer.cpp
    src/graphics/Shader.cpp
    src/scene/Camera.cpp
    src/scene/RenderObject.cpp
)
target_include_directories(AtmoGL PRIVATE src)
target_link_libraries(AtmoGL PRIVATE OpenGL::GL GLEW::GLEW GLUT::GLUT glm::glm)

if(MSVC)
    target_compile_options(AtmoGL PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(AtmoGL PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

add_custom_command(TARGET AtmoGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:AtmoGL>/shaders"
)
set_target_properties(AtmoGL PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AtmoGL>"
)
