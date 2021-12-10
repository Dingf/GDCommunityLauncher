#include <fstream>
#include <filesystem>
#include "TEXImage.h"
#include "ItemDBR.h"
#include "Log.h"

// List of different variable names that could point to the item bitmap
const std::vector<std::string> bitmapVariables =
{
    "bitmap",
    "artifactBitmap",
    "relicBitmap",
    "noteBitmap",
    "artifactFormulaBitmapName",
};

// List of directories to search for item DBRs
const std::vector<std::string> searchPaths =
{
    "records/items",
    "records/storyelements",      // For some reason Lokarr's set is stored here instead of in records/items
    "records/endlessdungeon/items",
    "records/endlessdungeon/scriptentities",
    "records/creatures/npcs/npcgear",
};

void ItemDBR::BuildItemDB(const std::filesystem::path& dataPath, const std::filesystem::path& outPath)
{
    if (!std::filesystem::is_directory(dataPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", dataPath.string().c_str()));

    if (!std::filesystem::is_directory(outPath))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "% is not a valid directory", outPath.string().c_str()));

    std::vector<std::unique_ptr<ItemDBR>> itemDBRs;
    for (uint32_t i = 0; i < searchPaths.size(); ++i)
    {
        std::filesystem::path searchPath = dataPath / searchPaths[i];
        for (auto& entry : std::filesystem::recursive_directory_iterator(searchPath))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".dbr"))
            {
                std::unique_ptr<ItemDBR> itemDBR = std::make_unique<ItemDBR>(entry.path());
                itemDBRs.push_back(std::move(itemDBR));
            }
        }
    }

    if (itemDBRs.empty())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not find any item DBRs in %", outPath.string().c_str()));

    std::filesystem::path outFilePath = outPath / "ItemDatabase.txt";

    std::ofstream out(outFilePath, std::ofstream::out);
    if (!out.is_open())
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "Could not open file % for writing", outFilePath.string().c_str()));

    //TODO: Also extract the item bitmaps and save them for later use?
    for (uint32_t i = 0; i < itemDBRs.size(); ++i)
    {
        uint32_t width = itemDBRs[i]->GetWidth();
        uint32_t height = itemDBRs[i]->GetHeight();
        if ((width > 0) && (height > 0))
        {
            std::string itemName = itemDBRs[i]->GetRecordPath().string();
            std::replace(itemName.begin(), itemName.end(), '\\', '/');
            out << "\"" << itemDBRs[i]->GetRecordPath().string() << "\" " << width / 32 << " " << height / 32 << "\n";
        }
    }

    out.close();
}

ItemDBR::ItemDBR(const std::filesystem::path& path) : _width(0), _height(0)
{
    if (!DBRecord::Load(path))
        throw std::runtime_error(Logger::LogMessage(LOG_LEVEL_ERROR, "The specified path is not a valid item DBR file"));

    const Value* bitmapPath = nullptr;
    for (uint32_t i = 0; i < bitmapVariables.size(); ++i)
    {
        bitmapPath = GetVariable(bitmapVariables[i]);
        if (bitmapPath)
            break;
    }

    if (bitmapPath)
    {
        std::string textureString = bitmapPath->ToString();
        if (textureString.front() == '/')
            textureString = std::string(textureString.begin() + 1, textureString.end());

        std::filesystem::path texturePath = GetRootPath() / textureString;
        if (std::filesystem::is_regular_file(texturePath))
        {
            TEXImage image(texturePath.string());

            _width = image.GetWidth();
            _height = image.GetHeight();
        }
        else
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Item DBR % refers to bitmap % which does not exist. Width/height data will not be loaded.", path, texturePath);
        }
    }
}