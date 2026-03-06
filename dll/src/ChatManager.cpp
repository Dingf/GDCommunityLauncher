#include <cwctype>
#include "EngineAPI.h"
#include "GameAPI.h"
#include "ChatManager.h"
#include "DllClient.h"
#include "EventManager.h"
#include "ThreadManager.h"
#include "Configuration.h"
#include "Item.h"
#include "StringConvert.h"
#include "URI.h"
#include "Log.h"

// TODO: Fix all of the connection stuff with the chat manager

/*void ChatManager::OnConnection(const signalr::value& value)
{
    if (!value.is_null())
    {
        ChatManager& chatManager = ChatManager::GetInstance();
        spChatManager->_connectionID = value.as_array()[0].as_string();
        GameAPI::AddChatMessage(L"Server", L"Connected to chat server.", EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatManager::OnReceiveMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        const auto& values = value.as_array();

        std::wstring name = RawToWide(values[0].as_string());
        std::wstring message = RawToWide(values[1].as_string());
        uint8_t type = (uint8_t)values[2].as_double();

        if (spChatManager->IsPlayerMuted(name))
            return;

        if (type == EngineAPI::UI::CHAT_TYPE_WHISPER)
            name = L"[From " + name + L"]";

        void* item = nullptr;
        if ((values.size() >= 4) && (values[3].as_string().size() > 0))
        {
            web::json::value itemJSON = web::json::value::parse(values[3].as_string());
            GameAPI::ItemReplicaInfo itemInfo = GameAPI::ItemToInfo(Item(itemJSON));
            itemInfo._itemID = EngineAPI::CreateObjectID();
            item = GameAPI::CreateItem(itemInfo);
        }

        GameAPI::AddChatMessage(name, message, type, item);
    }
}

void ChatManager::OnServerMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        const auto& values = value.as_array();
        std::wstring message = CharToWide(values[1].as_string());
        GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
    }
}

void ChatManager::OnWelcomeMessage(const signalr::value& value)
{
    if (!value.is_null())
    {
        const auto& values = value.as_array();
        const auto& messages = values[0].as_array();
        for (size_t i = 0; i < messages.size(); ++i)
        {
            std::wstring message = CharToWide(messages[i].as_string());
            GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_NORMAL);
        }

        spChatManager->DisplayNewTradeNotifications();
    }
}

void ChatManager::OnJoinedChannel(const signalr::value& value)
{
    if (!value.is_null())
    {
        const auto& values = value.as_array();

        uint32_t channels = (uint32_t)values[0].as_double();
        if ((channels & 0xF0) != (sChatManager._channels & 0xF0))
        {
            spChatManager->_channels = (spChatManager->_channels & 0x0F) | (channels & 0xF0);

            std::wstring joinMessage;
            if ((channels & 0xF0) == 0)
                joinMessage = L"Global chat is now OFF.";
            else
                joinMessage = L"Joined global channel " + std::to_wstring((channels & 0xF0) >> 4) + L".";

            GameAPI::AddChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_GLOBAL);
        }
        if ((channels & 0x0F) != (spChatManager->_channels & 0x0F))
        {
            spChatManager->_channels = (spChatManager->_channels & 0xF0) | (channels & 0x0F);

            std::wstring joinMessage;
            if ((channels & 0x0F) == 0)
                joinMessage = L"Trade chat is now OFF.";
            else
                joinMessage = L"Joined trade channel " + std::to_wstring(channels & 0x0F) + L".";

            GameAPI::AddChatMessage(L"Server", joinMessage, EngineAPI::UI::CHAT_TYPE_TRADE);
        }

        chatManager.SaveConfig();
    }
}

void ChatManager::OnBanned(const signalr::value& value)
{
    GameAPI::AddChatMessage(L"Server", L"Your account has been banned from chat.", EngineAPI::UI::CHAT_TYPE_NORMAL);
}*/

ChatManager::ChatManager()
{
    /*Register("Connection", OnConnection);
    Register("ReceiveMessage", OnReceiveMessage);
    Register("SystemMessage", OnServerMessage);
    Register("WelcomeMessage", OnWelcomeMessage);
    Register("JoinedChannel", OnJoinedChannel);
    Register("Banned", OnBanned);*/

    _channels = 0;
    _tradeColor = EngineAPI::Color::GREEN.GetColorCode();
    _globalColor = EngineAPI::Color::ORANGE.GetColorCode();
    _visible = nullptr;
    _colors = nullptr;
    _holdTime = 0;
    _holdEvent._key = EngineAPI::Input::KEY_NONE;

    EventManager::Subscribe(GDCL_EVENT_SHUTDOWN,         &ChatManager::OnShutdownEvent);
    //EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_LOAD,   &ChatManager::OnWorldPreLoadEvent);
    EventManager::Subscribe(GDCL_EVENT_WORLD_PRE_UNLOAD, &ChatManager::OnWorldPreUnloadEvent);
    EventManager::Subscribe(GDCL_EVENT_SET_MAIN_PLAYER,  &ChatManager::OnSetMainPlayerEvent);
    EventManager::Subscribe(GDCL_EVENT_KEY_BUTTON_EVENT, &ChatManager::OnKeyButtonEvent);

    _holdThread = std::make_unique<std::thread>([this](){ HoldThreadLoop(); });
}

ChatManager::~ChatManager()
{
    EventManager::Unsubscribe(GDCL_EVENT_SHUTDOWN,         &ChatManager::OnShutdownEvent);
    //EventManager::Unsubscribe(GDCL_EVENT_WORLD_PRE_LOAD,   &ChatManager::OnWorldPreLoadEvent);
    EventManager::Unsubscribe(GDCL_EVENT_WORLD_PRE_UNLOAD, &ChatManager::OnWorldPreUnloadEvent);
    EventManager::Unsubscribe(GDCL_EVENT_SET_MAIN_PLAYER,  &ChatManager::OnSetMainPlayerEvent);
    EventManager::Unsubscribe(GDCL_EVENT_KEY_BUTTON_EVENT, &ChatManager::OnKeyButtonEvent);

    if (_holdThread)
    {
        _holdTime = HOLD_THREAD_STOP; // This should have already been set in OnShutdownEvent(), but set it again just in case
        _holdThread->join();
        _holdThread.reset();
    }
}

ChatManager* ChatManager::GetInstance()
{
    static ChatManager instance;
    return &instance;
}

bool ChatManager::Initialize()
{
    try
    {
        GetInstance();
        return true;
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to initialize ChatManager module: %", ex.what());
        return false;
    }
}

uint32_t ChatManager::GetChatColor(ChatType type) const
{
    switch (type)
    {
        case CHAT_TYPE_TRADE:
            return _tradeColor;
        case CHAT_TYPE_GLOBAL:
            return _globalColor;
        default:
            return 0xFFFFFFFF;
    }
}

uint8_t ChatManager::GetChatChannel(ChatType type) const
{
    switch (type)
    {
        case CHAT_TYPE_GLOBAL:
            return (_channels & 0xF0) >> 4;
        case CHAT_TYPE_TRADE:
            return (_channels & 0x0F);
        default:
            return 0;
    }
}

bool ChatManager::SetChatColor(ChatType type, uint32_t color)
{
    if (_colors)
    {
        float* baseAddress = nullptr;

        // Trim alpha since we will always set it to 1.0f
        color &= 0x00FFFFFF;

        if (type == CHAT_TYPE_TRADE)
        {
            baseAddress = (float*)(_colors);
            _tradeColor = color;
        }
        else if (type == CHAT_TYPE_GLOBAL)
        {
            baseAddress = (float*)(_colors + sizeof(uint64_t) * 4);
            _globalColor = color;
        }

        if (baseAddress)
        {
            baseAddress[0] = (color & 0x0000FF) / 255.0f;
            baseAddress[1] = ((color & 0x00FF00) >> 8) / 255.0f;
            baseAddress[2] = ((color & 0xFF0000) >> 16) / 255.0f;
            baseAddress[3] = 1.0f;
        }
        return true;
    }
    return false;
}

void ChatManager::SetChatChannel(ChatType type, uint32_t channel)
{
    uint32_t oldChannel = _channels;
    uint32_t newChannel = channel;
    switch (type)
    {
        case CHAT_TYPE_NORMAL:
            newChannel = (uint8_t)(channel & 0xFF);
            break;
        case CHAT_TYPE_TRADE:
            newChannel = (oldChannel & 0xF0) | (channel & 0x0F);
            break;
        case CHAT_TYPE_GLOBAL:
            newChannel = (oldChannel & 0x0F) | ((channel & 0x0F) << 4);
            break;
    }

    /*if (IsConnected())
    {
        Client& client = Client::GetInstance();
        InvokeAsync("JoinChannel", GetConnectionID(), client.GetUsername(), oldChannel, newChannel);
    }*/
}

void ChatManager::SendChatMessage(ChatType type, const std::wstring& name, const std::wstring& message, void* item)
{
    uint32_t channelValue = 0;
    switch (type)
    {
        case CHAT_TYPE_GLOBAL:
            channelValue = _channels & 0xF0;
            break;
        case CHAT_TYPE_TRADE:
            channelValue = _channels & 0x0F;
            break;
    }

    json itemJSON;
    if (item)
    {
        GameAPI::ItemReplicaInfo itemInfo = GameAPI::GetItemReplicaInfo(item);
        itemJSON = InfoToItem(itemInfo);
    }

    //InvokeAsync("Send", WideToRaw(name), WideToRaw(message), channelValue, itemJSON);
}

void ChatManager::SetChannelAndSendMessage(ChatType type, uint32_t channel, const std::wstring& name, const std::wstring& message, void* item)
{
    //Client& client = Client::GetInstance();
    //std::vector<signalr::value> args;
    uint32_t oldChannel = _channels;
    uint32_t newChannel = channel;
    switch (type)
    {
        case CHAT_TYPE_NORMAL:
            newChannel = (uint8_t)(channel & 0xFF);
            break;
        case CHAT_TYPE_TRADE:
            newChannel = (oldChannel & 0xF0) | (channel & 0x0F);
            break;
        case CHAT_TYPE_GLOBAL:
            newChannel = (oldChannel & 0x0F) | ((channel & 0x0F) << 4);
            break;
    }

    //args.push_back(GetConnectionID());
    //args.push_back(client.GetUsername());
    //args.push_back((double)oldChannel);
    //args.push_back((double)newChannel);

    /*_connection->invoke("JoinChannel", args, [=](const signalr::value& value, std::exception_ptr ex)
    {
        try
        {
            if (ex)
                std::rethrow_exception(ex);

            SendChatMessage(type, name, message, item);
        }
        catch (const std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to join chat channel: %", ex.what());
        }
    });*/
}

void ChatManager::DisplayNewTradeNotifications()
{
    /*Client& client = Client::GetInstance();
    URI endpoint = client.GetServerGameURL() / "Trade" / "participant" / std::to_string(client.GetCurrentParticipantID()) / "trade-notifications" / "new";
    endpoint.AddParam("branch", client.GetBranchName());

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                std::wstring responseBody = response.extract_string().get();
                uint32_t newMessages = std::stoi(responseBody);
                if (newMessages > 0)
                {
                    std::wstring message = L"You have " + responseBody + L" new trade notification";
                    if (newMessages > 1)
                        message += L"s";
                    message += L".";
                    GameAPI::AddChatMessage(L"Server", message, EngineAPI::UI::CHAT_TYPE_TRADE);
                }
                return;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve new trade notifications: %", ex.what());
    }*/
}

bool ChatManager::MutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    if (_mutedList.count(playerName) == 0)
    {
        _mutedList.insert(playerName);
        return true;
    }
    else
    {
        return false;
    }
}

bool ChatManager::UnmutePlayer(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    if (_mutedList.count(playerName) > 0)
    {
        _mutedList.erase(playerName);
        return true;
    }
    else
    {
        return false;
    }
}

bool ChatManager::IsPlayerMuted(std::wstring playerName)
{
    std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
    return (_mutedList.count(playerName) > 0);
}

void ChatManager::LoadConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);

        const Value* channelValue = config.GetValue("Chat", "channel");
        SetChatChannel(CHAT_TYPE_NORMAL, (channelValue) ? channelValue->ToInt() : 0);

        const Value* tradeColorValue = config.GetValue("Chat", "trade_color");
        SetChatColor(CHAT_TYPE_TRADE, (tradeColorValue) ? tradeColorValue->ToInt() : EngineAPI::Color::GREEN.GetColorCode());

        const Value* globalColorValue = config.GetValue("Chat", "global_color");
        SetChatColor(CHAT_TYPE_GLOBAL, (globalColorValue) ? globalColorValue->ToInt() : EngineAPI::Color::ORANGE.GetColorCode());
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

void ChatManager::SaveConfig()
{
    Configuration config;
    std::filesystem::path configPath = std::filesystem::current_path() / "GDCommunityLauncher.ini";
    if (std::filesystem::is_regular_file(configPath))
    {
        config.Load(configPath);
        config.SetValue("Chat", "channel", (int)_channels);
        config.SetValue("Chat", "trade_color", (int)(_tradeColor & 0x00FFFFFF));
        config.SetValue("Chat", "global_color", (int)(_globalColor & 0x00FFFFFF));
        config.Save(configPath);
    }
    else
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Could not open configuration file %", configPath.string());
    }
}

uint32_t& ChatManager::GetCaratPosition()
{
    return *(uint32_t*)(_visible + 0x160);
}

uint32_t& ChatManager::GetSelectStartPosition()
{
    return *(uint32_t*)(_visible + 0x164);
}

uint32_t& ChatManager::GetSelectEndPosition()
{
    return *(uint32_t*)(_visible + 0x168);
}

const std::wstring& ChatManager::GetBufferText() const
{
    return *(std::wstring*)(_visible + 0xB0);
}

std::wstring& ChatManager::GetBufferText()
{
    return *(std::wstring*)(_visible + 0xB0);
}

void ChatManager::SetBufferText(const std::wstring& text)
{
    std::wstring trimmedText = (text.size() >= MAX_MESSAGE_SIZE) ? text.substr(0, MAX_MESSAGE_SIZE) : text;
    std::wstring& bufferText = GetBufferText();
    uint32_t& caratPosition = GetCaratPosition();

    bufferText = trimmedText;
    caratPosition = (uint32_t)trimmedText.size();
}

void ChatManager::FindMagicAddresses()
{
    void* gameEngine = *GameAPI::GetGameEngineHandle();
    if (gameEngine)
    {
        std::string versionString = EngineAPI::GetVersionString();

        //_visible = *(uint8_t**)((uint8_t*)gameEngine + 0x18A0) + 0x45F90;    // Pre-version 1.2.0.5
        //_visible = *(uint8_t**)((uint8_t*)gameEngine + 0x18B0) + 0x45BD8;    // Version 1.2.0.5
        _visible = *(uint8_t**)((uint8_t*)gameEngine + 0x18C0) + 0x45BD8;      // Version 1.2.1.3

        _colors = _visible + 0x2C28;

        // Change the in-game command names to avoid conflicts with the launcher chat commands
        *(std::wstring*)(_colors - 0x120) = L"_Mute";
        *(std::wstring*)(_colors - 0x100) = L"_Unmute";
        *(std::wstring*)(_colors - 0x80) = L"_t";
        *(std::wstring*)(_colors - 0x60) = L"_m";
        *(std::wstring*)(_colors - 0x40) = L"_u";
    }
    else
    {
        _visible = nullptr;
        _colors = nullptr;
    }
}

void ChatManager::ToggleWindowDisplay()
{
    if (_visible != nullptr)
    {
        if ((*_visible == 0) && (!_prefix.empty()))
            SetBufferText(_prefix);

        *_visible ^= 1;
        *(_visible + 0xA9) = *_visible;
    }
}

void ChatManager::HoldThreadLoop()
{
    while (_holdTime >= 0)
    {
        if (_holdEvent._key != EngineAPI::Input::KEY_NONE)
        {
            int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            if (now >= _holdTime)
                HandleKeyPress(_holdEvent);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void ChatManager::OnShutdownEvent()
{
    //spChatManager->Disconnect();
    spChatManager->_holdTime = HOLD_THREAD_STOP; // This breaks the loop in the hold thread, allowing us to join it
}

//void ChatManager::OnWorldPreLoadEvent(std::string mapName, bool modded)
//{

// Connect the client at game start; we can't do this when the DLL is loaded due to networking code
//if ((!spChatManager->IsConnected()) && (spChatManager->Connect()))
//    spChatManager->Invoke("GetConnectionId");
//}

void ChatManager::OnWorldPreUnloadEvent()
{
    spChatManager->_visible = nullptr;
    spChatManager->_colors = nullptr;
}

void ChatManager::OnSetMainPlayerEvent(void* player)
{
    spChatManager->FindMagicAddresses();
    spChatManager->LoadConfig();
    spChatManager->LoadMutedList();
}

// Special paste handler, since regular paste won't exceed the chat window length
bool HandlePasteEvent(std::wstring& text, uint32_t& carat, uint32_t& selectStart, uint32_t& selectEnd)
{
    if (OpenClipboard(nullptr))
    {
        if (HANDLE data = GetClipboardData(CF_TEXT))
        {
            if (char* charData = (char*)GlobalLock(data))
            {
                int32_t select = (selectEnd >= selectStart) ? selectEnd - selectStart : 0;
                size_t length = text.size() - select;
                std::wstring pasteText;
                while ((*charData != '\0') && (length < ChatManager::MAX_MESSAGE_SIZE))
                {
                    pasteText.push_back(*charData);
                    charData++;
                    length++;
                }

                if (select > 0)
                {
                    text.erase(selectStart, select);
                    carat = selectStart;
                    selectStart = 0;
                    selectEnd = 0;
                }

                text.insert(carat, pasteText);
                carat += (uint32_t)pasteText.size();

                GlobalUnlock(data);
                CloseClipboard();
                return true;
            }
        }
    }
    return false;
}

typedef bool (*ControlKeyHandler)(std::wstring&, uint32_t&, uint32_t&, uint32_t&);
const std::map<EngineAPI::Input::KeyCode, ControlKeyHandler> controlKeyHandlers =
{
    { EngineAPI::Input::KEY_V, &HandlePasteEvent },
};

bool ChatManager::HandleKeyPress(EngineAPI::Input::KeyButtonEvent& event)
{
    if (_visible != nullptr)
    {
        std::wstring& text = GetBufferText();
        uint32_t& carat = GetCaratPosition();
        uint32_t& selectStart = GetSelectStartPosition();
        uint32_t& selectEnd = GetSelectEndPosition();
        wchar_t output = (wchar_t)event._output;    // Cast to avoid endianness issues between different OS

        switch (event._key)
        {
            case EngineAPI::Input::KEY_TAB:
            case EngineAPI::Input::KEY_ESC:
                return false;
            case EngineAPI::Input::KEY_BACKSPACE:
                if (IsWindowVisible())
                {
                    int32_t select = selectEnd - selectStart;
                    if (select > 0)
                    {
                        text.erase(selectStart, select);
                        carat = selectStart;
                        selectStart = 0;
                        selectEnd = 0;
                    }
                    else if (carat > 0)
                    {
                        text.erase(--carat, 1);
                    }
                    return true;
                }
                return false;
            case EngineAPI::Input::KEY_ENTER:
            {
                if (!IsWindowVisible())
                {
                    ToggleWindowDisplay();
                    return true;
                }
                return false;
            }
            default:
            {
                if ((IsWindowVisible()) && (output != 0) && (text.size() < MAX_MESSAGE_SIZE))
                {
                    // Ctrl + key usually has output, but shouldn't actually print a character to the window
                    // Most of the time, just let the main program handle it
                    if (event._modifier & EngineAPI::Input::KEY_MODIFIER_CTRL)
                    {
                        auto pair = controlKeyHandlers.find(event._key);
                        if (pair != controlKeyHandlers.end())
                        {
                            ControlKeyHandler handler = pair->second;
                            return handler(text, carat, selectStart, selectEnd);
                        }
                        return false;
                    }

                    int32_t select = selectEnd - selectStart;
                    if (select > 0)
                    {
                        text.erase(selectStart, select);
                        carat = selectStart;
                        selectStart = 0;
                        selectEnd = 0;
                    }

                    text.insert(carat, 1, output);
                    carat++;
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}

bool ChatManager::OnKeyButtonEvent(EngineAPI::Input::KeyButtonEvent& event)
{
    if ((spClient->IsPlayingSeason()) && (!EngineAPI::IsMultiplayer()))
    {
        if (event._state == EngineAPI::Input::KEY_STATE_DOWN)
        {
            // This needs to be set here because the hold event also calls HandleKeyPress() to handle the repeated inputs
            spChatManager->_holdEvent = event;
            spChatManager->_holdTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + 500;
            return spChatManager->HandleKeyPress(event);
        }
        else
        {
            spChatManager->_holdEvent._key = EngineAPI::Input::KEY_NONE;
        }
    }
    return false;
}

void ChatManager::LoadMutedList()
{
    /*Client& client = Client::GetInstance();
    URI endpoint = client.GetServerChatURL() / "chat" / "mute-list";

    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    std::string bearerToken = "Bearer " + client.GetAuthToken();
    request.headers().add(U("Authorization"), bearerToken.c_str());

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get(); 
            web::json::array mutedList = responseBody.as_array();

            for (size_t i = 0; i < mutedList.size(); ++i)
            {
                // Strip quotes and convert to lower-case before adding the player name
                std::wstring playerName = mutedList[i].serialize();
                playerName = std::wstring(playerName.begin() + 1, playerName.end() - 1);
                std::transform(playerName.begin(), playerName.end(), playerName.begin(), std::towlower);
                spChatManager->_mutedList.insert(playerName);
            }
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve muted list: %", ex.what());
    }*/
}