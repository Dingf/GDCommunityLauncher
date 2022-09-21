cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project (GDCommunityLauncher VERSION 1.1.4)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CMAKE_CXX_STANDARD 17)
set(GDCL_BUILD_EXTRACTOR ON)

configure_file(${CMAKE_SOURCE_DIR}/common/include/Version.cmake ${CMAKE_SOURCE_DIR}/common/include/Version.h)

find_package(minizip CONFIG REQUIRED)
if (minizip_FOUND)
    include_directories(${minizip_INCLUDE_DIR})
endif()

find_package(minhook CONFIG REQUIRED)
if (minhook_FOUND)
    find_path(minhook_INCLUDE_DIR MinHook.h)
    include_directories(${minhook_INCLUDE_DIR})
endif()

find_package(cpprestsdk CONFIG REQUIRED)
if (cpprestsdk_FOUND)
    find_path(cpprestsdk_INCLUDE_DIR cpprest/json.h)
    include_directories(${cpprestsdk_INCLUDE_DIR})
endif()

find_package(microsoft-signalr CONFIG REQUIRED)

include_directories(common/include)
include_directories(${CMAKE_SOURCE_DIR})

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

add_subdirectory(common)
add_subdirectory(game)
add_subdirectory(exe)
add_subdirectory(dll)

if (${GDCL_BUILD_EXTRACTOR})
    find_package(lz4 CONFIG REQUIRED)
    if (lz4_FOUND)
        find_path(lz4_INCLUDE_DIR lz4.h)
        find_library(lz4_LIBRARY lz4.lib)
    endif()
    add_subdirectory(extractor)
endif()