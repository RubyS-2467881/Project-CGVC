cmake_minimum_required(VERSION 3.14)
project(Project-CGVC)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your local 'includes' folder to the include path
include_directories(${CMAKE_SOURCE_DIR}/includes)

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
)

# Make sure glad.c uses the right includes
target_include_directories(Project-CGVC PRIVATE ${CMAKE_SOURCE_DIR}/includes)

# ---- Link Libraries ----
target_link_libraries(Project-CGVC glfw)

# ---- Windows-specific OpenGL linking ----
if (WIN32)
    target_link_libraries(Project-CGVC opengl32)
endif()
