include_directories(include)
include_directories(../common/include)

add_executable(exe WIN32
include/Client.h
include/GameLauncher.h
include/LoginPrompt.h
res/LoginPrompt.rc
src/Client.cpp
src/GameLauncher.cpp
src/main.cpp)

target_compile_definitions(exe INTERFACE GDCL_EXE=1)

set_target_properties(exe PROPERTIES OUTPUT_NAME "GDCommunityLauncher")