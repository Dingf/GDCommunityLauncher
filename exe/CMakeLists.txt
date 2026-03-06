file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE RESOURCE "res/*.rc")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_executable(exe WIN32 ${INCLUDE} ${RESOURCE} ${SOURCE})

target_include_directories(exe PRIVATE include)
target_link_libraries(exe PRIVATE cpprestsdk::cpprest)
target_link_libraries(exe PRIVATE microsoft-signalr::microsoft-signalr)
target_link_libraries(exe PRIVATE OpenSSL::SSL)
if (USE_OLD_CMAKEFINDS)
	target_link_libraries(exe PRIVATE minizip::minizip)
else()
	target_link_libraries(exe PRIVATE unofficial::minizip::minizip)
endif()
target_link_libraries(exe PRIVATE common)

set_target_properties(exe PROPERTIES
    OUTPUT_NAME "GDCommunityLauncher"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)