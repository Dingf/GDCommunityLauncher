add_executable(exe WIN32
include/Client.h
include/GameLauncher.h
include/LoginDialog.h
include/ServerAuth.h
res/LoginDialog.rc
src/Client.cpp
src/GameLauncher.cpp
src/LoginDialog.cpp
src/ServerAuth.cpp
src/main.cpp)

target_include_directories(exe PRIVATE include)
target_link_libraries(exe PRIVATE Boost::thread)

set_target_properties(exe PROPERTIES OUTPUT_NAME "GDCommunityLauncher")