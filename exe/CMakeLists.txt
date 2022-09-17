add_executable(exe WIN32
include/Client.h
include/Configuration.h
include/GameLauncher.h
include/LoginDialog.h
include/UpdateDialog.h
res/LoginDialog.rc
res/UpdateDialog.rc
src/Client.cpp
src/Configuration.cpp
src/GameLauncher.cpp
src/LoginDialog.cpp
src/UpdateDialog.cpp
src/main.cpp)

target_include_directories(exe PRIVATE include)
target_link_libraries(exe PRIVATE minizip::minizip)
target_link_libraries(exe PRIVATE common)
target_link_libraries(exe PRIVATE dll)

set_target_properties(exe PROPERTIES OUTPUT_NAME "GDCommunityLauncher")