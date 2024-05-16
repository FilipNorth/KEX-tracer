cmake_minimum_required(VERSION 3.14)
project(KEXtracer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include GLFW
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
add_subdirectory(${PROJECT_SOURCE_DIR}/Libraries/lib/glfw)

# Add the GLAD source
set(GLAD_SOURCE "${PROJECT_SOURCE_DIR}/Libraries/include/glad/glad.c")  # Adjust this if glad.c is elsewhere

# Source and header files
file(GLOB_RECURSE SOURCES src/*.cpp ${GLAD_SOURCE})  # Include glad.c here
file(GLOB_RECURSE HEADERS src/*.h ${PROJECT_SOURCE_DIR}/Libraries/include/glad/glad.h)  # Ensure glad.h is included

# Define the executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/Libraries/include  # This should encompass all necessary headers
    ${PROJECT_SOURCE_DIR}/Models/Sponza-glTF
    ${PROJECT_SOURCE_DIR}/Shaders
)

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glfw)

# Compiler options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)  # Using /WX treats warnings as errors, remove if problematic
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Set the output directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
