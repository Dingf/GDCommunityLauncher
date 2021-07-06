#include <stdint.h>
#include "Configuration.h"
#include "FileReader.h"

std::string ReadBufferedString(const char* buffer, int64_t start, int64_t end)
{
    if (end > start)
    {
        std::string s;
        s.reserve(end - start);
        for (int64_t i = start; i < end; ++i)
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
        std::shared_ptr<FileReader> reader = FileReader::Open(path);

        int64_t bufferSize = reader->GetBufferSize();
        const char* buffer = (const char*)reader->GetBuffer();

        std::string key;
        std::string section;
        int64_t lineStart = 0;
        int64_t valueStart = -1;
        int64_t sectionStart = -1;
        for (int64_t i = 0; i < bufferSize; ++i)
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
    std::ofstream out(path, std::ofstream::out);
    if (!out.is_open())
        return false;

    std::string currentSection = {};
    for (auto it = _configValues.begin(); it != _configValues.end(); ++it)
    {
        ConfigKey key = it->first;
        std::unique_ptr<Value>& value = it->second;

        if (key.first != currentSection)
        {
            out << "[" << key.first << "]\n";
            currentSection = key.first;
        }

        out << key.second << "=";
        switch (value->GetType())
        {
            case VALUE_TYPE_INT:
                out << value->ToInt() << "\n";
                break;
            case VALUE_TYPE_FLOAT:
                out << value->ToFloat() << "\n";
                break;
            case VALUE_TYPE_BOOL:
                out << (value->ToBool() ? "true" : "false") << "\n";
                break;
            case VALUE_TYPE_STRING:
                out << value->ToString() << "\n";
                break;
            default:
                return false;
        }
    }

    out.close();
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