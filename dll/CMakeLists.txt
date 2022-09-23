add_library(dll SHARED
include/EventManager.h
include/HookManager.h
include/UpdateThread.h
include/EngineAPI.h
include/EngineAPI/Platform.h
include/EngineAPI/Color.h
include/EngineAPI/Rect.h
include/EngineAPI/GraphicsAlign.h
include/EngineAPI/KeyButtonEvent.h
include/EngineAPI/UI/ChatWindow.h
include/GameAPI.h
include/GameAPI/Difficulty.h
include/GameAPI/TriggerToken.h
include/LuaAPI.h
include/ChatClient.h
include/Client.h
include/ClientHandlers.h
include/DatabaseValues.h
res/ItemDatabaseValues.rc
res/DungeonDatabaseValues.rc
src/EventManager.cpp
src/HookManager.cpp
src/EngineAPI.cpp
src/EngineAPI/Platform.cpp
src/EngineAPI/Color.cpp
src/EngineAPI/UI/ChatWindow.cpp
src/GameAPI.cpp
src/GameAPI/TriggerToken.cpp
src/LuaAPI.cpp
src/ChatClient.cpp
src/Client.cpp
src/ClientHandlers/HandleDisplayText.cpp
src/ClientHandlers/HandleRender.cpp
src/ClientHandlers/HandleWorld.cpp
src/ClientHandlers/HandleKeyEvent.cpp
src/ClientHandlers/HandleLua.cpp
src/ClientHandlers/HandleSetMainPlayer.cpp
src/ClientHandlers/HandleSaveNewFormatData.cpp
src/ClientHandlers/HandleTransferStash.cpp
src/ClientHandlers/HandleBestowToken.cpp
src/ClientHandlers/HandleSendChatMessage.cpp
src/ClientHandlers/HandleEndlessDungeon.cpp
src/main.cpp)

target_include_directories(dll PRIVATE include)
target_link_libraries(dll PRIVATE cpprestsdk::cpprest cpprestsdk::cpprestsdk_zlib_internal cpprestsdk::cpprestsdk_brotli_internal)
target_link_libraries(dll PRIVATE microsoft-signalr::microsoft-signalr)
target_link_libraries(dll PRIVATE minizip::minizip)
target_link_libraries(dll PRIVATE minhook::minhook)
target_link_libraries(dll PRIVATE ${LUA_LIBRARY})
target_link_libraries(dll PRIVATE game)

set_target_properties(dll PROPERTIES OUTPUT_NAME "GDCommunityLauncher")