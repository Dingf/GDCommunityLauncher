add_library(dll SHARED
include/HookManager.h
include/EngineAPI.h
include/EngineAPI/Color.h
include/EngineAPI/GraphicsAlign.h
include/EngineAPI/KeyButtonEvent.h
include/GameAPI.h
include/GameAPI/Difficulty.h
include/Client.h
src/HookManager.cpp
src/EngineApi.cpp
src/EngineAPI/Color.cpp
src/GameAPI.cpp
src/Client.cpp
src/ClientHooks.cpp
src/main.cpp)

target_include_directories(dll PRIVATE include)
target_link_libraries(dll PRIVATE game)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")