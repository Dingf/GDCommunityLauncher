#ifndef INC_GDCL_EXTRACTOR_ITEM_DBR_H
#define INC_GDCL_EXTRACTOR_ITEM_DBR_H

#include <vector>
#include <string>
#include "ItemType.h"
#include "DBRecord.h"

class ItemDBR : public DBRecord
{
    public:
        ItemDBR(const std::filesystem::path& path, bool loadBitmap = true);

        static const std::vector<std::string>& GetSearchPaths() { return _searchPaths; }

        static void BuildItemDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath);

        ItemType GetType() const { return _type; }
        uint32_t GetWidth() const { return _width; }
        uint32_t GetHeight() const { return _height; }

    private:
        static const std::vector<std::string> _bitmapVariables;
        static const std::vector<std::string> _searchPaths;

        ItemType _type;
        uint32_t _width;
        uint32_t _height;
};

#endif//INC_GDCL_EXTRACTOR_ITEM_DBR_H