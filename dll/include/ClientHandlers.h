#ifndef INC_GDCL_DLL_CLIENT_HANDLERS_H
#define INC_GDCL_DLL_CLIENT_HANDLERS_H

#include <string>
#include <vector>
#include <filesystem>
#include "HookManager.h"
#include "EngineAPI.h"
#include "GameAPI.h"
#include "Client.h"

const char* HandleGetVersion(void* _this);
void HandleRender(void* _this);
bool HandleKeyEvent(void* _this, EngineAPI::KeyButtonEvent& event);
bool HandleLoadWorld(void* _this, const char* mapName, bool unk1, bool modded);
void HandleSetRegionOfNote(void* _this, void* region);
void HandleRenderStyledText2D(void* _this, const EngineAPI::Rect& rect, const wchar_t* text, const std::string& style, float unk1, EngineAPI::GraphicsXAlign xAlign, EngineAPI::GraphicsYAlign yAlign, int layout);
void HandleLuaInitialize(void* _this, bool unk1, bool unk2);
void HandleSetMainPlayer(void* _this, uint32_t unk1);
void HandleSetTransferOpen(void* _this, uint32_t unk1, bool unk2, bool unk3);
void HandleSaveNewFormatData(void* _this, void* writer);
void HandleSaveTransferStash(void* _this);
void HandleLoadTransferStash(void* _this);
void HandleBestowToken(void* _this, const GameAPI::TriggerToken& token);
void HandleUnloadWorld(void* _this);
void HandleSendChatMessage(void* _this, const std::wstring& name, const std::wstring& message, uint8_t type, std::vector<uint32_t> targets, uint32_t unk1);
void HandleSyncDungeonProgress(void* _this, int unk1);

std::string GenerateFileMD5(const std::filesystem::path& path);

#endif//INC_GDCL_DLL_CLIENT_HANDLERS_H