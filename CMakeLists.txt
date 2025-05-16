cmake_minimum_required(VERSION 3.14)
project(Project-CGVC)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your local 'includes' folder to the include path
include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories(${CMAKE_SOURCE_DIR}/camera)
include_directories(${CMAKE_SOURCE_DIR}/shaders)
include_directories(${CMAKE_SOURCE_DIR}/assets)
include_directories(${CMAKE_SOURCE_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# ---- Fetch GLFW using FetchContent ----
include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# ---- Source files ----
add_executable(Project-CGVC
    main.cpp
    glad.c
 "glm.h" "camera/camera.h" "camera/cameraDirection.h"  "shaders/shader.h" "stb.cpp")

# Make sure glad.c uses the right includes
target_include_directories(Project-CGVC PRIVATE ${CMAKE_SOURCE_DIR}/includes)

# ---- Link Libraries ----
target_link_libraries(Project-CGVC glfw)

# ---- Windows-specific OpenGL linking ----
if (WIN32)
    target_link_libraries(Project-CGVC opengl32)
endif()
