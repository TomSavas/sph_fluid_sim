cmake_minimum_required(VERSION 3.1)

set(PROJECT sph_fluid_sim)
project(${PROJECT})
set(EXEC ${PROJECT})

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb -O2 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

file(GLOB_RECURSE SOURCES "src/*.cpp" "lib/imgui/*.cpp")
message(${SOURCES})

add_executable(${EXEC} ${SOURCES})

target_include_directories(${PROJECT} PRIVATE src/)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/glfw)
target_link_libraries(${PROJECT} glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT} OpenGL::GL)

add_subdirectory(lib/glm)
target_link_libraries(${PROJECT} glm::glm)

target_link_libraries(${PROJECT} GLEW)

target_include_directories(${PROJECT} PRIVATE lib/stb)

target_include_directories(${PROJECT} PRIVATE lib/imgui)

add_definitions(-DGLFW_INCLUDE_NONE)
