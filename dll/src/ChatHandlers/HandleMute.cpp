#include <string>
#include "ChatAPI.h"
#include "JSON.h"
#include "Log.h"

std::string HandleWriteGetMutedList(uint32_t requestID)
{
    json request = 
    {
        { "RequestName", "MuteList" },
        { "RequestId", requestID },
    };
    return request.dump();
}

std::string HandleWriteMutePlayer(uint32_t requestID, std::wstring& playerName)
{
    json request = 
    {
        { "RequestName", "Mute" },
        { "RequestId", requestID },
        { "Message", playerName }
    };
    return request.dump();
}

std::string HandleWriteUnmutePlayer(uint32_t requestID, std::wstring& playerName)
{
    json request = 
    {
        { "RequestName", "Unmute" },
        { "RequestId", requestID },
        { "Message", playerName }
    };
    return request.dump();
}

void HandleReadGetMutedList(const json& response)
{
    ChatAPI::ClearMutedList();

    const json& message = response.at("Message");
    for (const json& player : message)
    {
        std::wstring playerName = player.get<std::wstring>();
        ChatAPI::MutePlayer(playerName);
    }
}

void HandleReadMutePlayer(const json& response, std::wstring playerName)
{
    // TODO: Possibly need to handle extra cases like the player name not existing on the server or is already muted
    std::string message = response.at("Message").get<std::string>();
    if (message == "Ok")
        ChatAPI::MutePlayer(playerName);
}

void HandleReadUnmutePlayer(const json& response, std::wstring playerName)
{
    // TODO: Possibly need to handle extra cases like the player name not existing on the server or is already muted
    std::string message = response.at("Message").get<std::string>();
    if (message == "Ok")
        ChatAPI::UnmutePlayer(playerName);
}