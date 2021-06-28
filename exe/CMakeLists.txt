add_executable(exe WIN32
include/Client.h
include/Configuration.h
include/GameLauncher.h
include/LoginDialog.h
include/ServerAuth.h
res/LoginDialog.rc
src/Client.cpp
src/Configuration.cpp
src/GameLauncher.cpp
src/LoginDialog.cpp
src/ServerAuth.cpp
src/main.cpp)

target_include_directories(exe PRIVATE include)

set_target_properties(exe PROPERTIES OUTPUT_NAME "GDCommunityLauncher")