file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE RESOURCE "res/*.rc")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_library(dll SHARED ${INCLUDE} ${RESOURCE} ${SOURCE})

target_include_directories(dll PRIVATE include)
target_link_libraries(dll PRIVATE cpprestsdk::cpprest)
target_link_libraries(dll PRIVATE microsoft-signalr::microsoft-signalr)
target_link_libraries(dll PRIVATE minizip::minizip)
target_link_libraries(dll PRIVATE minhook::minhook)
target_link_libraries(dll PRIVATE ${LUA_LIBRARY})
target_link_libraries(dll PRIVATE game)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")