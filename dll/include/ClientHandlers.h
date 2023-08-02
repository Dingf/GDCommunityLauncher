#ifndef INC_GDCL_DLL_CLIENT_HANDLERS_H
#define INC_GDCL_DLL_CLIENT_HANDLERS_H

#include <string>
#include <vector>
#include "HookManager.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "Client.h"
#include "Log.h"

const char* HandleGetVersion(void* _this);
bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event);
bool HandleMouseEvent(void* _this, EngineAPI::MouseEvent& event);
bool HandleDirectRead(void* _this, const std::string& filename, void*& data, uint32_t& size, bool unk1, bool unk2);
bool HandleDirectWrite(void* _this, const std::string& filename, void* data, uint32_t size, bool unk1, bool unk2, bool unk3);
bool HandleLoadWorld(void* _this, const char* mapName, bool unk1, bool modded);
void HandleSetRegionOfNote(void* _this, void* region);
void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::UI::GraphicsXAlign xAlign, EngineAPI::UI::GraphicsYAlign yAlign, int layout);
void HandleLuaInitialize(void* _this, bool unk1, bool unk2);
int32_t HandleCreateNewConnection(void* _this, void* unk1, void* unk2, void* unk3);
void HandleAddNetworkServer(void* _this, void* server, uint32_t unk1);
void HandleGameShutdown(void* _this);
void HandleSetMainPlayer(void* _this, uint32_t unk1);
void HandleSetTransferOpen(void* _this, uint32_t unk1, bool unk2, bool unk3);
void HandleSaveNewFormatData(void* _this, void* writer);
void HandleLoadNewFormatData(void* _this, void* reader);
void HandleSaveTransferStash(void* _this);
void HandleLoadTransferStash(void* _this);
void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token);
void HandleUnloadWorld(void* _this);
void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t itemID);
void HandleSyncDungeonProgress(void* _this, int unk1);
bool HandleUseItemEnchantment(void* _this, void* item, bool unk1, bool& unk2);
bool HandleCanEnchantBeUsedOn(void* _this, void* item, bool unk1, bool& unk2);
void HandleGetItemDescription(void* _this, std::vector<GameAPI::GameTextLine>& lines);
void HandleGetItemDescriptionWeapon(void* _this, std::vector<GameAPI::GameTextLine>& lines);
void HandleGetItemDescriptionArmor(void* _this, std::vector<GameAPI::GameTextLine>& lines);
std::string HandleGetRootSavePath();
std::string HandleGetBaseFolder(void* _this);
std::string& HandleGetUserSaveFolder(void* _this, void* unk1);
std::string& HandleGetFullSaveFolder(void* _this, void* unk1, void* player);
std::string& HandleGetPlayerFolder1(void* _this, void* unk1, const std::string& playerName, bool unk2, bool unk3);
std::string& HandleGetPlayerFolder2(void* _this, void* unk1, void* player);
std::string& HandleGetMapFolder(void* _this, void* unk1, const std::string& name, void* player);
std::string& HandleGetDifficultyFolder(void* _this, void* unk1, GameAPI::Difficulty difficulty, const std::string& mapName, void* player);
void HandleGetSharedSavePath(void* _this, uint32_t type, std::string& path, bool unk1, bool unk2, bool unk3);

#endif//INC_GDCL_DLL_CLIENT_HANDLERS_H