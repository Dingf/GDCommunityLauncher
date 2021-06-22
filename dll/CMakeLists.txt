include_directories(include)
include_directories(../common/include)
include_directories(../game/include)

add_library(dll SHARED
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

add_dependencies(dll game)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")