#ifndef INC_GDCL_EXTRACTOR_DB_RECORD_H
#define INC_GDCL_EXTRACTOR_DB_RECROD_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "Value.h"

enum DBDataType
{
    DB_DATA_TYPE_INT = 0,
    DB_DATA_TYPE_FLOAT = 1,
    DB_DATA_TYPE_STRING = 2,
};

class DBRecord
{
    public:
        virtual ~DBRecord() = 0;

        const Value* GetVariable(const std::string& key, uint32_t index = 0);

        std::string GetRootPath() const { return _rootPath; }
        std::string GetRecordPath() const { return _recordPath; }

        bool Load(const std::string& filename);

    protected:
        std::map<std::string, std::vector<std::unique_ptr<Value>>> _variables;

        std::string _rootPath;
        std::string _recordPath;
};

#endif//INC_GDCL_EXTRACTOR_DB_RECORD_H