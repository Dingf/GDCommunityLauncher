#ifndef INC_GDCL_EXTRACTOR_DB_RECORD_H
#define INC_GDCL_EXTRACTOR_DB_RECORD_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <filesystem>
#include "Value.h"

class DBRecord
{
    public:
        virtual ~DBRecord() = 0;

        const Value* GetVariable(const std::string& key, uint32_t index = 0);

        std::filesystem::path GetRootPath()   const { return _rootPath; }
        std::filesystem::path GetRecordPath() const { return _recordPath; }

        virtual bool Load(const std::filesystem::path& path);

    protected:
        std::map<std::string, std::vector<std::unique_ptr<Value>>> _variables;

        std::filesystem::path _rootPath;
        std::filesystem::path _recordPath;
};

#endif//INC_GDCL_EXTRACTOR_DB_RECORD_H