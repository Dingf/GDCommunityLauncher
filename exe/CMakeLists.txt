file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE RESOURCE "res/*.rc")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_executable(exe WIN32 ${INCLUDE} ${RESOURCE} ${SOURCE})

target_include_directories(exe PRIVATE include)
target_link_libraries(exe PRIVATE OpenSSL::SSL)
target_link_libraries(exe PRIVATE $<IF:$<TARGET_EXISTS:MINIZIP::minizip>,MINIZIP::minizip,MINIZIP::minizipstatic>)
target_link_libraries(exe PRIVATE common)

if (WIN32)
    target_link_libraries(exe PRIVATE gdiplus)
    target_link_libraries(exe PRIVATE shlwapi)
endif()

set_target_properties(exe PROPERTIES
    OUTPUT_NAME "GDCommunityLauncher"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)