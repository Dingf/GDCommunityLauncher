#ifndef INC_GDCL_EXTRACTOR_ITEM_DBR_H
#define INC_GDCL_EXTRACTOR_ITEM_DBR_H

#include "DBRecord.h"

class ItemDBR : public DBRecord
{
    public:
        ItemDBR(const std::filesystem::path& path);

        static void BuildItemDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath);

        uint32_t GetWidth() const { return _width; }
        uint32_t GetHeight() const { return _height; }

    private:

        uint32_t _width;
        uint32_t _height;
};

#endif//INC_GDCL_EXTRACTOR_ITEM_DBR_H