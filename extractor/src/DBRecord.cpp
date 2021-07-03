#include <algorithm>
#include "DBRecord.h"

DBRecord::~DBRecord() {}

const Value* DBRecord::GetVariable(const std::string& key, uint32_t index)
{
    if ((_variables.count(key) == 0) || (index >= _variables[key].size()))
        return nullptr;

    return _variables[key][index].get();
}

bool DBRecord::Load(const std::string& filename)
{
    FILE * file;
    if (fopen_s(&file, filename.c_str(), "r") != 0)
        return false;

    fseek(file, 0, SEEK_END);
    uint32_t bufferSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = new char[bufferSize];
    fread(buffer, 1, bufferSize, file);
    fclose(file);

    std::string filenameCopy(filename);
    std::replace(filenameCopy.begin(), filenameCopy.end(), '\\', '/');

    uint32_t pathStart = filenameCopy.find("/records/");
    if (pathStart < filenameCopy.length())
    {
        _rootPath = filenameCopy.substr(0, pathStart);
        _recordPath = filenameCopy.substr(pathStart + 1);
    }
    else
        return false;

    uint32_t bufferPos = 0;
    while (bufferPos < bufferSize)
    {
        uint32_t recordStart = bufferPos;
        uint32_t dataStart = 0;
        uint32_t dataEnd = 0;

        while ((bufferPos < bufferSize) && (buffer[bufferPos] != '\n'))
        {
            bufferPos++;
            if (buffer[bufferPos] == ',')
            {
                if (dataStart == 0)
                    dataStart = bufferPos;
                else
                    dataEnd = bufferPos;
            }
        }

        if ((bufferPos >= bufferSize) || (dataStart == 0))
            break;

        std::string key(&buffer[recordStart], dataStart - recordStart);

        int i;
        float f;
        uint32_t dataCurrent = dataStart + 1;
        while (dataCurrent <= dataEnd)
        {
            char c = buffer[dataCurrent];
            if ((c == ';') || (c == ','))
            {
                std::string val(&buffer[dataStart + 1], dataCurrent - dataStart - 1);
                _variables[key].emplace_back(val);
                dataStart = dataCurrent++;
            }
            else
            {
                dataCurrent++;
            }
        }
        bufferPos++;
    }

    delete[] buffer;

    return true;
}