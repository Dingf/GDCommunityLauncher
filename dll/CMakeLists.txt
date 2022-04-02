add_library(dll SHARED
include/HookManager.h
include/UpdateThread.h
include/EngineAPI.h
include/EngineAPI/Color.h
include/EngineAPI/Rect.h
include/EngineAPI/GraphicsAlign.h
include/EngineAPI/KeyButtonEvent.h
include/GameAPI.h
include/GameAPI/Difficulty.h
include/Client.h
include/ClientHandlers.h
include/ItemDatabaseValues.h
res/ItemDatabaseValues.rc
src/HookManager.cpp
src/EngineApi.cpp
src/EngineAPI/Color.cpp
src/GameAPI.cpp
src/Client.cpp
src/ClientHandlers/HandleGetVersion.cpp
src/ClientHandlers/HandleRender.cpp
src/ClientHandlers/HandleWorld.cpp
src/ClientHandlers/HandleKeyEvent.cpp
src/ClientHandlers/HandleLua.cpp
src/ClientHandlers/HandleSetMainPlayer.cpp
src/ClientHandlers/HandleSaveNewFormatData.cpp
src/ClientHandlers/HandleTransferStash.cpp
src/ClientHandlers/HandleBestowToken.cpp
src/ClientHandlers/HandleSendChatMessage.cpp
src/main.cpp)

target_include_directories(dll PRIVATE include)
target_link_libraries(dll PRIVATE minizip::minizip)
target_link_libraries(dll PRIVATE minhook::minhook)
target_link_libraries(dll PRIVATE game)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")