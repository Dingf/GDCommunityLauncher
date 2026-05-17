#include "Reagents.h"
#include "Log.h"

size_t Reagents::GetBufferSize() const
{
    size_t size = 28 + _headerBlock._modName.size();

    for (size_t i = 0; i < _headerBlock._items.size(); ++i)
    {
        size += 20 + _headerBlock._items[i]->_name.size();
    }

    return size;
}

bool Reagents::ReadFromFile(const std::filesystem::path& path)
{
    if (std::filesystem::is_regular_file(path))
    {
        try
        {
            EncodedFileReader reader(path);
            if (!reader.HasData())
                throw std::runtime_error(std::string("Could not open file: \"") + path.string().c_str() + "\" for reading");

            Read(&reader);
            return true;
        }
        catch (std::runtime_error& ex)
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load reagents file \"%\": %", path.string().c_str(), ex.what());
            return false;
        }
    }
    return false;
}

bool Reagents::ReadFromBuffer(const uint8_t* data, size_t size)
{
    try
    {
        EncodedFileReader reader(data, size);
        Read(&reader);
        return true;
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to load reagents data: %", ex.what());
        return false;
    }
}

bool Reagents::WriteToFile(const std::filesystem::path& path)
{
    try
    {
        EncodedFileWriter writer(GetBufferSize());
        Write(&writer);
        writer.WriteToFile(path);
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write to reagents file \"%\": %", path.string().c_str(), ex.what());
        return false;
    }
    return true;
}

bool Reagents::WriteToBuffer(uint8_t* data, size_t size)
{
    try
    {
        EncodedFileWriter writer(GetBufferSize());
        Write(&writer);
        memcpy(data, writer.GetBuffer(), std::min(size, GetBufferSize()));
    }
    catch (std::runtime_error& ex)
    {
        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write reagents data: %", ex.what());
        return false;
    }
    return true;
}

void Reagents::Read(EncodedFileReader* reader)
{
    uint32_t fileVersion = reader->ReadInt32(true);
    if (fileVersion != 1)
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The file version is invalid or unsupported"));

    _headerBlock.ReadBlockStart(reader);

    _headerBlock._unk1 = reader->ReadInt32(false);
    _headerBlock._modName = reader->ReadString();

    _headerBlock._items.clear();
    uint32_t count = reader->ReadInt32();
    for (uint32_t i = 0; i < count; ++i)
    {
        std::unique_ptr<ReagentsItemBlock> itemBlock = std::make_unique<ReagentsItemBlock>();

        itemBlock->ReadBlockStart(reader, GD_DATA_BLOCK_FLAG_ID);

        itemBlock->_name = reader->ReadString();
        itemBlock->_count = reader->ReadInt32();

        itemBlock->ReadBlockEnd(reader);
        _headerBlock._items.push_back(std::move(itemBlock));
    }

    _headerBlock.ReadBlockEnd(reader);
}

void Reagents::Write(EncodedFileWriter* writer)
{
    writer->BufferInt32(1);

    _headerBlock.WriteBlockStart(writer);

    writer->BufferInt32(0);
    writer->BufferString(_headerBlock._modName);

    uint32_t count = (uint32_t)_headerBlock._items.size();
    writer->BufferInt32(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        std::unique_ptr<ReagentsItemBlock>& itemBlock = _headerBlock._items[i];
        itemBlock->WriteBlockStart(writer, GD_DATA_BLOCK_FLAG_ID);

        writer->BufferString(itemBlock->_name);
        writer->BufferInt32(itemBlock->_count);

        itemBlock->WriteBlockEnd(writer);
    }

    _headerBlock.WriteBlockEnd(writer);
}

void to_json(json& j, const Reagents& data)
{
    j = json
    {
        { "HeaderBlock", data._headerBlock },
    };
}

void from_json(const json& j, Reagents& data)
{
    j.at("HeaderBlock").get_to(data._headerBlock);
}

void to_json(json& j, const Reagents::ReagentsHeaderBlock& data)
{
    json items;
    for (size_t i = 0; i < data._items.size(); ++i)
    {
        items.push_back(*data._items[i]);
    }

    j = json
    {
        { "BlockID",      data.GetBlockID() },
        { "BlockVersion", data.GetBlockVersion() },
        { "ModName",      data._modName },
        { "Items",        items },
    };
}

void from_json(const json& j, Reagents::ReagentsHeaderBlock& data)
{
    data._items.clear();

    j.at("ModName").get_to(data._modName);

    json items = j.at("Items");
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        std::unique_ptr<Reagents::ReagentsItemBlock> itemBlock = std::make_unique<Reagents::ReagentsItemBlock>();
        it->get_to(*itemBlock);
        data._items.push_back(std::move(itemBlock));
    }
}

void to_json(json& j, const Reagents::ReagentsItemBlock& data)
{
    j = json
    {
        { "Name",       data._name },
        { "StackCount", data._count },
    };
}

void from_json(const json& j, Reagents::ReagentsItemBlock& data)
{
    j.at("Name")      .get_to(data._name);
    j.at("StackCount").get_to(data._count);
}