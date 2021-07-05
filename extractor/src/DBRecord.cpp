#include <algorithm>
#include "DBRecord.h"
#include "FileReader.h"

DBRecord::~DBRecord() {}

const Value* DBRecord::GetVariable(const std::string& key, uint32_t index)
{
    if ((_variables.count(key) == 0) || (index >= _variables[key].size()))
        return nullptr;

    return _variables[key][index].get();
}

bool DBRecord::Load(const std::filesystem::path& path)
{
    // Split the given path into root and record sections
    // The root path is used to generate absolute paths when loading files and the record path
    // is used internally by the DBRs to reference other files
    _rootPath.clear();
    _recordPath.clear();

    bool recordStart = false;
    for (auto it = path.begin(); it != path.end(); ++it)
    {
        if (recordStart)
            _recordPath /= *it;
        else
            _rootPath /= *it;

        if ((!recordStart) && (*it == "records"))
            recordStart = true;
    }

    if ((_rootPath.empty()) || (_recordPath.empty()))
        return false;

    std::shared_ptr<FileReader> reader = FileReader::Open(path);
    const char* buffer = (const char*)reader->GetBuffer();

    // DBR files are plaintext so loop through each line and parse all of the record data
    int64_t bufferPos = 0;
    int64_t bufferSize = reader->GetBufferSize();
    while (bufferPos < bufferSize)
    {
        int64_t recordStart = bufferPos;
        int64_t dataStart = 0;
        int64_t dataEnd = 0;

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

        uint32_t dataCurrent = dataStart + 1;
        while (dataCurrent <= dataEnd)
        {
            char c = buffer[dataCurrent];
            if ((c == ';') || (c == ','))
            {
                std::string val(&buffer[dataStart + 1], dataCurrent - dataStart - 1);
                _variables[key].push_back(Value::Parse(val));
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