#include "Stash.h"
#include "Log.h"

Stash::~Stash()
{
    _stashTabs.clear();
}

size_t Stash::GetBufferSize() const
{
    size_t size = 4;
    for (uint32_t i = 0; i < _stashTabs.size(); ++i)
    {
        size += 8;
        ItemContainerType containerType = GetContainerType();
        switch (containerType)
        {
            case ITEM_CONTAINER_SHARED_STASH:
            case ITEM_CONTAINER_CHAR_STASH:
                size += 8;
                break;
            case ITEM_CONTAINER_CHAR_BAG:
                size++;
                break;
            default:
                throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Invalid or unsupported item container type \"%\"", containerType));
        }

        size += 4;
        const ItemContainer::ItemList& itemList = _stashTabs[i]->_stashTab->GetItemList();
        for (auto it = itemList.begin(); it != itemList.end(); ++it)
        {
            size += it->first->GetBufferSize();
            size += 8;
        }
        size += 4;
    }

    return size;
}

void Stash::ReadStashTabs(EncodedFileReader* reader, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        std::unique_ptr<StashTabBlock> stashTabBlock = std::make_unique<StashTabBlock>();

        uint32_t tabWidth = 0;
        uint32_t tabHeight = 0;

        stashTabBlock->ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_ID);

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

        // This has been observed but it's unclear what causes it, how to reproduce it, or if the bug still exists
        // So just keep this here to prevent people from losing all stash data if it happens
        if ((tabWidth == 0) || (tabHeight == 0))
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Stash tab % has invalid dimensions (width = %, height = %). Using default stash tab dimensions instead.", i, tabWidth, tabHeight);
            tabWidth = 10;
            tabHeight = 18;
        }

        std::unique_ptr<StashTab> stashTab(new StashTab(*this, tabWidth, tabHeight));

        uint16_t itemX, itemY;
        uint32_t numItems = reader->ReadInt32();
        for (uint32_t j = 0; j < numItems; ++j)
        {
            Item item(reader);
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

            if (!stashTab->AddItem(item, itemX, itemY))
                Logger::LogMessage(LOG_LEVEL_WARN, "Could not add item \"%\" at coordinates (%, %).", item._itemName, itemX, itemY);
        }

        if (stashTab->GetItemCount() != numItems)
            Logger::LogMessage(LOG_LEVEL_WARN, "The number of items read from the container (%) does not match the expected number of items (%)", stashTab->GetItemCount(), numItems);

        stashTabBlock->ReadBlockEnd(reader);

        stashTabBlock->_stashTab = std::move(stashTab);
        _stashTabs.push_back(std::move(stashTabBlock));
    }
}

void Stash::WriteStashTabs(EncodedFileWriter* writer)
{
    for (uint32_t i = 0; i < _stashTabs.size(); ++i)
    {
        _stashTabs[i]->WriteBlockStart(writer, GD_DATA_BLOCK_FLAG_ID);

        ItemContainerType containerType = GetContainerType();
        switch (containerType)
        {
            case ITEM_CONTAINER_SHARED_STASH:
            case ITEM_CONTAINER_CHAR_STASH:
                writer->BufferInt32(_stashTabs[i]->_stashTab->_width);
                writer->BufferInt32(_stashTabs[i]->_stashTab->_height);
                break;
            case ITEM_CONTAINER_CHAR_BAG:
                writer->BufferInt8(0);
                break;
            default:
                throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Invalid or unsupported item container type \"%\"", containerType));
        }

        const ItemContainer::ItemList& itemList = _stashTabs[i]->_stashTab->GetItemList();
        writer->BufferInt32((uint32_t)itemList.size());
        for (auto pair : itemList)
        {
            uint32_t itemX = (pair.second >> 32) & 0xFFFFFFFF;
            uint32_t itemY = (pair.second & 0xFFFFFFFF);

            pair.first->Write(writer);
            if (containerType == ITEM_CONTAINER_CHAR_BAG)
            {
                writer->BufferInt32(itemX);
                writer->BufferInt32(itemY);
            }
            else
            {
                writer->BufferFloat((float)itemX);
                writer->BufferFloat((float)itemY);
            }
        }

        _stashTabs[i]->WriteBlockEnd(writer);
    }
}

Stash::StashTab* Stash::GetStashTab(size_t index)
{
    if (index < _stashTabs.size())
        return _stashTabs[index]->_stashTab.get();
    return nullptr;
}

web::json::value Stash::ToJSON() const
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

web::json::value Stash::StashTabBlock::ToJSON() const
{
    web::json::value obj = web::json::value::object();

    if (_stashTab)
    {
        uint32_t i = 0;
        web::json::value items = web::json::value::array();
        for (auto pair : _stashTab->GetItemList())
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