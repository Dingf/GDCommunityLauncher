#ifndef INC_GDCL_GAME_ITEM_H
#define INC_GDCL_GAME_ITEM_H

#include <memory>
#include <string>
#include "FileData.h"
#include "JSONObject.h"

struct Item : public FileData, public JSONObject
{
    public:
        Item() {}
        Item(EncodedFileReader* reader) { Read(reader); }
        Item(const web::json::value& val);
        Item(const Item& item) { *this = item; }

        Item& operator=(const Item& item);

        web::json::value ToJSON() const;

        void Read(EncodedFileReader* reader);
        void Write(EncodedFileWriter* writer);

        size_t GetBufferSize() const;

        std::string _itemName;
        std::string _itemPrefix;
        std::string _itemSuffix;
        std::string _itemModifier;
        std::string _itemIllusion;
        std::string _itemComponent;
        std::string _itemCompletion;
        std::string _itemAugment;

        uint32_t _itemID;
        uint32_t _itemSeed;
        uint32_t _itemComponentSeed;
        uint32_t _itemUnk1;
        uint32_t _itemAugmentSeed;
        uint32_t _itemUnk2;
        uint32_t _itemStackCount;

        uint32_t _itemWidth;
        uint32_t _itemHeight;
};

#endif//INC_GDCL_GAME_ITEM_H