#include <stdint.h>
#include <stdio.h>
#include "Configuration.h"

std::string ReadBufferedString(const char* buffer, uint32_t start, uint32_t end)
{
    if (end > start)
    {
        std::string s;
        s.reserve(end - start);
        for (uint32_t i = start; i < end; ++i)
        {
            s.push_back((buffer[i] == '\\') ? buffer[++i] : buffer[i]);
        }
        return s;
    }
    return {};
}

bool Configuration::Load(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path) && (path.extension() == ".ini"))
    {
        FILE* file;
#if _WIN32
        if (_wfopen_s(&file, path.c_str(), L"rb") != 0)
#else
        if (fopen_s(&file, path.c_str(), "rb") != 0)
#endif
        {
            return false;
        }

        fseek(file, 0, SEEK_END);
        int32_t bufferSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char* buffer = new char[bufferSize];
        fread(buffer, 1, bufferSize, file);
        fclose(file);

        std::string key;
        std::string section;
        int32_t lineStart = 0;
        int32_t valueStart = -1;
        int32_t sectionStart = -1;
        for (int32_t i = 0; i < bufferSize; ++i)
        {
            switch (buffer[i])
            {
                case '[':
                {
                    if ((i != lineStart) || (sectionStart != -1))
                        return false;

                    sectionStart = i + 1;
                    break;
                }
                case ']':
                {
                    if (sectionStart == -1)
                        return false;

                    section = ReadBufferedString(buffer, sectionStart, i);
                    sectionStart = -1;
                    break;
                }
                case '\r':
                case '\n':
                {
                    if (sectionStart != -1)
                        return false;

                    if (valueStart != -1)
                    {
                        std::string value = ReadBufferedString(buffer, valueStart, i);
                        _configValues.emplace(std::make_pair(section, key), Value::Parse(value));
                        valueStart = -1;
                    }
                    lineStart = i + 1;
                    break;
                }
                case ' ':
                case '\t':
                {
                    if (i == lineStart)
                        lineStart++;
                    break;
                }
                case '\\':
                {
                    i++;
                    break;
                }
                case '#':
                case ';':
                {
                    if (i == lineStart)
                    {
                        while ((i < bufferSize) && (buffer[i] != '\n'))
                            i++;
                        lineStart = i + 1;
                    }
                    break;
                }
                case '=':
                {
                    if ((valueStart == -1) && (sectionStart == -1))
                    {
                        key = ReadBufferedString(buffer, lineStart, i);
                        valueStart = i + 1;
                        break;
                    }
                    else
                        return false;
                }
            }
        }

        if (valueStart != -1)
        {
            std::string value = ReadBufferedString(buffer, valueStart, bufferSize);
            _configValues.emplace(std::make_pair(section, key), Value::Parse(value));
        }

        return true;
    }
    return false;
}

bool Configuration::Save(const std::filesystem::path& path)
{
    FILE* file;
#if _WIN32
    if (_wfopen_s(&file, path.c_str(), L"w") != 0)
#else
    if (fopen_s(&file, path.c_str(), "w") != 0)
#endif
    {
        return false;
    }

    std::string currentSection = {};
    for (auto it = _configValues.begin(); it != _configValues.end(); ++it)
    {
        ConfigKey key = it->first;
        std::unique_ptr<Value>& value = it->second;

        if (key.first != currentSection)
        {
            fprintf(file, "[%s]\n", key.first.c_str());
            currentSection = key.first;
        }

        fprintf(file, "%s=", key.second.c_str());
        switch (value->GetType())
        {
            case VALUE_TYPE_INT:
                fprintf(file, "%d\n", value->ToInt());
                break;
            case VALUE_TYPE_FLOAT:
                fprintf(file, "%f\n", value->ToFloat());
                break;
            case VALUE_TYPE_BOOL:
                fprintf(file, "%s\n", (value->ToBool()) ? "true" : "false");
                break;
            case VALUE_TYPE_STRING:
                fprintf(file, "%s\n", value->ToString());
                break;
            default:
                return false;
        }
    }

    fclose(file);
    return true;
}


const Value* Configuration::GetValue(const std::string& section, const std::string& name)
{
    ConfigKey key({ section, name });
    auto it = _configValues.find(key);
    if (it == _configValues.end())
        return nullptr;
    else
        return it->second.get();
}