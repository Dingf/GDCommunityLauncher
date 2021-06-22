include_directories(include)

add_library(dll SHARED
include/Log.h
include/HookManager.h
include/EngineAPI.h
include/GameAPI.h
include/Client.h
src/HookManager.cpp
src/EngineApi.cpp
src/GameAPI.cpp
src/Client.cpp
src/ClientHooks.cpp
src/main.cpp)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")