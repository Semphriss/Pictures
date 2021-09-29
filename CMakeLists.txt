
project(pictures)
cmake_minimum_required(VERSION 3.0)
add_executable(pictures src/main.cpp)

set(HARBOR_BUILD_EXEC OFF CACHE BOOL "" FORCE)
set(HARBOR_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(HARBOR_USE_VIDEO_OPENGL OFF CACHE BOOL "" FORCE)
add_subdirectory(external/harbor)
target_link_libraries(pictures PUBLIC harbor_lib)
target_include_directories(pictures PUBLIC external/harbor/src)
