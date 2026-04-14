#include <unordered_map>
#include "ChatAPI.h"
#include "EngineAPI.h"

#include "Log.h"

// Special paste handler, since regular paste won't exceed the default chat window length
bool HandlePasteEvent(std::wstring text, uint32_t carat, uint32_t selectStart, uint32_t selectEnd)
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
                while ((*charData != '\0') && (length < ChatAPI::MAX_MESSAGE_SIZE))
                {
                    pasteText.push_back(*charData);
                    charData++;
                    length++;
                }

                if (select > 0)
                {
                    text.erase(selectStart, select);
                    carat = selectStart;
                }

                text.insert(carat, pasteText);
                carat += (uint32_t)pasteText.size();

                ChatAPI::SetBufferTextDirect(text);
                ChatAPI::SetCaratPosition(carat);
                ChatAPI::SetSelectStartPosition(0);
                ChatAPI::SetSelectEndPosition(0);

                GlobalUnlock(data);
                CloseClipboard();
                return true;
            }
        }
    }
    return false;
}

typedef bool (*ControlKeyHandler)(std::wstring, uint32_t, uint32_t, uint32_t);
const std::unordered_map<EngineAPI::Input::KeyCode, ControlKeyHandler> controlKeyHandlers =
{
    { EngineAPI::Input::KEY_V, &HandlePasteEvent },
};

bool HandleCustomKeyEvent(EngineAPI::Input::KeyButtonEvent& event)
{
    if (ChatAPI::HasChatWindow())
    {
        std::wstring text = ChatAPI::GetBufferText();
        uint32_t carat = ChatAPI::GetCaratPosition();
        uint32_t selectStart = ChatAPI::GetSelectStartPosition();
        uint32_t selectEnd = ChatAPI::GetSelectEndPosition();
        wchar_t output = (wchar_t)event._output;    // Cast to avoid endianness issues between different OS

        switch (event._key)
        {
            case EngineAPI::Input::KEY_TAB:
            case EngineAPI::Input::KEY_ESC:
                return false;
            case EngineAPI::Input::KEY_BACKSPACE:
            {
                if (ChatAPI::IsWindowVisible())
                {
                    int32_t select = selectEnd - selectStart;
                    if (select > 0)
                    {
                        text.erase(selectStart, select);
                        ChatAPI::SetBufferTextDirect(text);
                        ChatAPI::SetCaratPosition(selectStart);
                        ChatAPI::SetSelectStartPosition(0);
                        ChatAPI::SetSelectEndPosition(0);
                    }
                    else if (carat > 0)
                    {
                        text.erase(--carat, 1);
                        ChatAPI::SetBufferTextDirect(text);
                        ChatAPI::SetCaratPosition(carat);
                    }
                    return true;
                }
                return false;
            }
            case EngineAPI::Input::KEY_ENTER:
            {
                if (!ChatAPI::IsWindowVisible())
                {
                    ChatAPI::ToggleWindowDisplay();
                    return true;
                }
                return false;
            }
            default:
            {
                if ((ChatAPI::IsWindowVisible()) && (output != 0))
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

                    // Need to return true here to prevent the default key handler from truncating the message
                    // However, don't actually print anything if it's over the max message size
                    if (text.size() >= ChatAPI::MAX_MESSAGE_SIZE)
                        return true;

                    int32_t select = selectEnd - selectStart;
                    if (select > 0)
                    {
                        text.erase(selectStart, select);
                        carat = selectStart;
                    }

                    text.insert(carat, 1, output);
                    ChatAPI::SetBufferTextDirect(text);
                    ChatAPI::SetCaratPosition(carat + 1);
                    ChatAPI::SetSelectStartPosition(0);
                    ChatAPI::SetSelectEndPosition(0);
                    return true;
                }
                return false;
            }
        }
    }
    return false;
}