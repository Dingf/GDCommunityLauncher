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

        if (containerType == ITEM_CONTAINER_CHAR_STASH)
        {
            CharacterStashTab* charTab = dynamic_cast<CharacterStashTab*>(_stashTabs[i]->_stashTab.get());
            size += 20;
            size += charTab->_buttonName.size() * 2;
        }

        size += 4;
    }

    return size;
}

void Stash::ReadStashTabs(EncodedFileReader* reader, uint32_t version, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        std::unique_ptr<StashTabBlock> stashTabBlock = std::make_unique<StashTabBlock>();

        uint32_t width = 0;
        uint32_t height = 0;

        stashTabBlock->ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_ID);

        ItemContainerType type = GetContainerType();
        switch (type)
        {
            case ITEM_CONTAINER_SHARED_STASH:
            case ITEM_CONTAINER_CHAR_STASH:
                width = reader->ReadInt32();
                height = reader->ReadInt32();
                break;
            case ITEM_CONTAINER_CHAR_BAG:
                reader->ReadInt8();                             // Inventory bag sizes aren't defined in the save, so hardcode them I guess?
                width = (_stashTabs.size() == 0) ? 12 : 8;   // First/main inventory bag is larger than the other ones
                height = 8;
                break;
            default:
                throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Invalid or unsupported item container type \"%\"", type));
        }

        // This has been observed frequently but it's unclear what causes it or how to reproduce it
        // So just keep this here to prevent people from losing all stash data if it happens
        if ((width == 0) || (height == 0))
        {
            width = 10;
            height = 18;
        }

        std::unique_ptr<StashTab> stashTab;
        if (GetContainerType() == ITEM_CONTAINER_CHAR_STASH)
            stashTab = std::make_unique<CharacterStashTab>(width, height);
        else
            stashTab = std::make_unique<StashTab>(type, width, height);

        uint16_t itemX, itemY;
        uint32_t numItems = reader->ReadInt32();
        for (uint32_t j = 0; j < numItems; ++j)
        {
            ItemReplicaInfo item(reader, version);
            if (type == ITEM_CONTAINER_CHAR_BAG)
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
                Logger::LogMessage(LOG_LEVEL_WARN, "Could not add item \"%\" at coordinates (%, %).", item._name, itemX, itemY);
        }

        if (stashTab->GetItemCount() != numItems)
            Logger::LogMessage(LOG_LEVEL_WARN, "The number of items read from the container (%) does not match the expected number of items (%)", stashTab->GetItemCount(), numItems);

        if ((GetContainerType() == ITEM_CONTAINER_CHAR_STASH) && (version >= 10))
        {
            CharacterStashTab* charTab = dynamic_cast<CharacterStashTab*>(stashTab.get());
            charTab->_borderIndex = reader->ReadInt32();
            charTab->_borderColorIndex = reader->ReadInt32();
            charTab->_symbolIndex = reader->ReadInt32();
            charTab->_symbolColorIndex = reader->ReadInt32();
            charTab->_buttonName = reader->ReadWideString();
        }

        stashTabBlock->ReadBlockEnd(reader);

        stashTabBlock->_stashTab = std::move(stashTab);
        _stashTabs.push_back(std::move(stashTabBlock));
    }
}

void Stash::WriteStashTabs(EncodedFileWriter* writer, uint32_t version)
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

        if ((GetContainerType() == ITEM_CONTAINER_CHAR_STASH) && (version >= 10))
        {
            CharacterStashTab* charTab = dynamic_cast<CharacterStashTab*>(_stashTabs[i]->_stashTab.get());
            writer->BufferInt32(charTab->_borderIndex);
            writer->BufferInt32(charTab->_borderColorIndex);
            writer->BufferInt32(charTab->_symbolIndex);
            writer->BufferInt32(charTab->_symbolColorIndex);
            writer->BufferWideString(charTab->_buttonName);
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

void to_json(json& j, const Stash& data)
{
    json stashTabs;
    for (size_t i = 0; i < data._stashTabs.size(); ++i)
    {
        json tab = *data._stashTabs[i];
        tab["ID"] = i;
        stashTabs.push_back(tab);
    }

    j = 
    {
        { "ContainerType", data.GetContainerType() },
        { "Hardcore",      data._isHardcore },
        { "Tabs",          stashTabs },
    };
}

void from_json(const json& j, Stash& data)
{
    data._stashTabs.clear();

    j.at("Hardcore").get_to(data._isHardcore);
    
    json stashTabs = j.at("Tabs");
    for (auto it = stashTabs.begin(); it != stashTabs.end(); ++it)
    {
        std::unique_ptr<Stash::StashTabBlock> stashTabBlock = std::make_unique<Stash::StashTabBlock>();
        it->get_to(*stashTabBlock);
        data._stashTabs.push_back(std::move(stashTabBlock));
    }
}


void to_json(json& j, const Stash::StashTabBlock& data)
{
    if (!data._stashTab)
    {
        j.clear();
        return;
    }

    json items;
    for (auto pair : data._stashTab->GetItemList())
    {
        json item = *pair.first;
        item["X"] = ((pair.second >> 32) & 0xFFFFFFFF);
        item["Y"] = (pair.second & 0xFFFFFFFF);
        items.push_back(item);
    }

    j = 
    {
        { "Width",  data._stashTab->GetWidth() },
        { "Height", data._stashTab->GetHeight() },
        { "Type",   data._stashTab->GetContainerType()},
        { "Items",  items }
    };
}

void from_json(const json& j, Stash::StashTabBlock& data)
{
    uint32_t width = j.at("Width");
    uint32_t height = j.at("Height");
    ItemContainerType type = j.at("Type");

    std::unique_ptr<Stash::StashTab> stashTab(new Stash::StashTab(type, width, height));

    json items = j.at("Items");
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        ItemReplicaInfo item;
        it->get_to(item);

        uint32_t itemX = it->at("X");
        uint32_t itemY = it->at("Y");

        if (!stashTab->AddItem(item, itemX, itemY))
            Logger::LogMessage(LOG_LEVEL_WARN, "Could not add item \"%\" at coordinates (%, %).", item._name, itemX, itemY);
    }

    if (stashTab->GetItemCount() != items.size())
        Logger::LogMessage(LOG_LEVEL_WARN, "The number of items read from the container (%) does not match the expected number of items (%)", stashTab->GetItemCount(), items.size());

    data._stashTab = std::move(stashTab);
}