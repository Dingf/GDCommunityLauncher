#ifndef INC_GDCL_DLL_CHAT_API_H
#define INC_GDCL_DLL_CHAT_API_H

#include <string>
#include <unordered_set>

namespace ChatAPI
{

enum ChatType : uint8_t
{
    CHAT_TYPE_NORMAL = 0,
    CHAT_TYPE_SYSTEM = 1,
    CHAT_TYPE_GLOBAL = 2,
    CHAT_TYPE_WHISPER = 3,
};

static constexpr uint32_t CHAT_CHANNEL_MAX = 255;
static constexpr uint32_t MAX_MESSAGE_SIZE = 255;

uint32_t GetChatColor(ChatType type);
uint8_t  GetChatChannel();

const std::wstring& GetChatPrefix();
const std::wstring& GetBufferText();
const std::wstring& GetSavedText();

uint32_t GetCaratPosition();
uint32_t GetSelectStartPosition();
uint32_t GetSelectEndPosition();

const std::unordered_set<std::wstring>& GetMutedList();

bool IsPlayerMuted(std::wstring playerName);
bool IsWindowVisible();

bool SetChatColor(ChatType type, uint32_t color);
void SetChatChannel(uint8_t channel);

void SetCaratPosition(uint32_t position);
void SetSelectStartPosition(uint32_t position);
void SetSelectEndPosition(uint32_t position);

void SetChatPrefix(const std::wstring& prefix);
void SetBufferText(const std::wstring& text);
void SaveBufferText();

bool MutePlayer(std::wstring playerName);
bool UnmutePlayer(std::wstring playerName);
void ClearMutedList();

void ToggleWindowDisplay();

bool Initialize();

}


#endif//INC_GDCL_DLL_CHAT_API_H