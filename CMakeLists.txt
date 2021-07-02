project (GDCommunityLauncher)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CXX_STANDARD 17)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(cpprestsdk REQUIRED)
if (cpprestsdk_FOUND)
    find_path(cpprestsdk_INCLUDE_DIR cpprest/json.h)
    find_library(cpprestsdk_LIBRARY cpprest_2_10.lib)
    include_directories(${cpprestsdk_INCLUDE_DIR})
endif()

find_package(Boost 1.75.0)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBARY_DIRS})
    add_definitions(-DBOOST_ALL_NO_LIB)
endif()

include_directories(common/include)

if (WIN32 AND CMAKE_SYSTEM_VERSION)
    string(REGEX MATCH "^([0-9]+).([0-9]).([0-9]+)" version ${CMAKE_SYSTEM_VERSION})
    set(major ${CMAKE_MATCH_1})
    set(minor ${CMAKE_MATCH_2})

    if (${major} MATCHES "^1([0-9])")
        string(HEX ${CMAKE_MATCH_1} digit)
        math(EXPR digit "0x${digit} + 17")
        string(ASCII ${digit} digit)
        set(major ${digit})
    endif()

    set(winnt_version "0x0${major}0${minor}")
    add_definitions(-D_WIN32_WINNT=${winnt_version})
endif()

add_subdirectory(game)
add_subdirectory(exe)
add_subdirectory(dll)