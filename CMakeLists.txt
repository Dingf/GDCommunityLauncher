project (GDCommunityLauncher)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_GENERATOR_PLATFORM x64)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(game)
add_subdirectory(exe)
add_subdirectory(dll)