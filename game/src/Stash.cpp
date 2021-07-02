#include "Stash.h"
#include "Log.h"

Stash::~Stash()
{
    _stashTabs.clear();
}

void Stash::ReadStashTab(EncodedFileReader* reader)
{
    std::unique_ptr<StashTabBlock> stashTabBlock = std::make_unique<StashTabBlock>();

    uint32_t tabWidth = 0;
    uint32_t tabHeight = 0;

    stashTabBlock->ReadBlockStart(reader, GD_DATA_BLOCK_READ_ID);

    ItemContainerType containerType = GetContainerType();
    switch (containerType)
    {
        case ITEM_CONTAINER_SHARED_STASH:
        case ITEM_CONTAINER_CHAR_STASH:
            tabWidth = reader->ReadInt32();
            tabHeight = reader->ReadInt32();
            break;
        case ITEM_CONTAINER_CHAR_BAG:
            reader->ReadInt8();                             // Inventory bag sizes aren't defined in the save, so hardcode them I guess?
            tabWidth = (_stashTabs.size() == 0) ? 12 : 8;   // First/main inventory bag is larger than the other ones
            tabHeight = 8;
            break;
        default:
            throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Invalid or unsupported item container type \"%\"", containerType));
    }

    std::unique_ptr<StashTab> stashTab(new StashTab(*this, tabWidth, tabHeight));

    uint16_t itemX, itemY;
    uint32_t numItems = reader->ReadInt32();
    for (uint32_t j = 0; j < numItems; ++j)
    {
        std::shared_ptr<Item> item = std::make_shared<Item>(reader);
        if (containerType == ITEM_CONTAINER_CHAR_BAG)
        {
            itemX = (uint16_t)reader->ReadInt32();
            itemY = (uint16_t)reader->ReadInt32();
        }
        else
        {
            itemX = (uint16_t)reader->ReadFloat();
            itemY = (uint16_t)reader->ReadFloat();
        }
        stashTab->AddItem(item, itemX, itemY);
    }

    if (stashTab->GetItemCount() != numItems)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The number of items read from the container (%) does not match the expected number of items (%)", stashTab->GetItemCount(), numItems));

    stashTabBlock->ReadBlockEnd(reader);

    stashTabBlock->_stashTab = std::move(stashTab);
    _stashTabs.push_back(std::move(stashTabBlock));
}

web::json::value Stash::ToJSON()
{
    web::json::value obj = web::json::value::object();

    obj[U("ContainerType")] = GetContainerType();
    obj[U("Hardcore")] = _isHardcore;

    web::json::value stashTabs = web::json::value::array();
    for (uint32_t i = 0; i < _stashTabs.size(); ++i)
    {
        web::json::value tabObject = _stashTabs[i]->ToJSON();
        tabObject[U("ID")] = i;
        stashTabs[i] = tabObject;
    }
    obj[U("Tabs")] = stashTabs;

    return obj;
}

web::json::value Stash::StashTabBlock::ToJSON()
{
    web::json::value obj = web::json::value::object();

    if (_stashTab)
    {
        uint32_t i = 0;
        web::json::value items = web::json::value::array();
        for (std::pair<Item*, uint64_t> pair : _stashTab->GetItemList())
        {
            web::json::value item = pair.first->ToJSON();
            item[U("X")] = ((pair.second >> 32) & 0xFFFFFFFF);
            item[U("Y")] = (pair.second & 0xFFFFFFFF);
            items[i++] = item;
        }
        obj[U("Items")] = items;
    }

    return obj;
}