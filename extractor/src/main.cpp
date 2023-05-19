#include <iostream>
#include <filesystem>
#include "ItemDBR.h"
#include "CraftingDBR.h"
#include "ARZExtractor.h"
#include "ARCExtractor.h"
#include "Log.h"

int main(int argc, char** argv)
{
    std::filesystem::path inputPath;
    std::filesystem::path outputPath;

    std::string modName;

    if ((argc == 2) || (argc == 4))
    {
        outputPath = argv[1];

        if (argc == 4)
        {
            inputPath = argv[2];
            modName = argv[3];
        }
    }
    else
    {
        std::string inputDir;
        std::string outputDir;

        std::cout << "Enter the output directory: ";
        std::getline(std::cin, outputDir);

        outputPath = outputDir;
        if (!std::filesystem::is_directory(outputPath))
        {
            if (std::filesystem::exists(outputPath))
            {
                std::cout << "Could not create output directory: the specified path already exists on the filesystem" << std::endl;
                return -1;
            }
            std::filesystem::create_directories(outputDir);
        }

        std::cout << "Enter the Grim Dawn directory (leave empty to skip extraction): ";
        std::getline(std::cin, inputDir);

        inputPath = inputDir;
        if (!inputPath.empty())
        {
            if (!std::filesystem::is_directory(inputPath))
            {
                std::cout << "The specified path is not a valid directory" << std::endl;
                return -1;
            }

#ifdef _WIN64
            std::filesystem::path grimDawnPath = inputPath / "x64" / "Grim Dawn.exe";
#else
            std::filesystem::path grimDawnPath = inputPath / "Grim Dawn.exe";
#endif
            if (!std::filesystem::is_regular_file(grimDawnPath))
            {
                std::cout << "The specified directory does not contain a Grim Dawn installation" << std::endl;
                return -1;
            }

            std::cout << "Enter the mod name: ";
            std::getline(std::cin, modName);
        }
    }

    Logger::SetLogFilename("GDCLExtractor.log");
    try
    {
        if (!inputPath.empty())
        {
            std::cout << "Extracting Grim Dawn records..." << std::endl;
            ARZExtractor::Extract(inputPath / "database" / "database.arz", outputPath);
            std::cout << "Extracting Grim Dawn text..." << std::endl;
            ARCExtractor::Extract(inputPath / "resources" / "Text_EN.arc", outputPath);
            std::cout << "Extracting Grim Dawn items..." << std::endl;
            ARCExtractor::Extract(inputPath / "resources" / "Items.arc", outputPath);
            std::cout << "Extracting Ashes of Malmouth records..." << std::endl;
            ARZExtractor::Extract(inputPath / "gdx1" / "database" / "GDX1.arz", outputPath);
            std::cout << "Extracting Ashes of Malmouth text..." << std::endl;
            ARCExtractor::Extract(inputPath / "gdx1" / "resources" / "Text_EN.arc", outputPath);
            std::cout << "Extracting Ashes of Malmouth items..." << std::endl;
            ARCExtractor::Extract(inputPath / "gdx1" / "resources" / "Items.arc", outputPath);
            std::cout << "Extracting Forgotten Gods records..." << std::endl;
            ARZExtractor::Extract(inputPath / "gdx2" / "database" / "GDX2.arz", outputPath);
            std::cout << "Extracting Forgotten Gods text..." << std::endl;
            ARCExtractor::Extract(inputPath / "gdx2" / "resources" / "Text_EN.arc", outputPath);
            std::cout << "Extracting Forgotten Gods items..." << std::endl;
            ARCExtractor::Extract(inputPath / "gdx2" / "resources" / "Items.arc", outputPath);
            std::cout << "Extracting GrimLeague records..." << std::endl;
            ARZExtractor::Extract(inputPath / "mods" / modName / "database" / (modName + ".arz"), outputPath);
            std::cout << "Extracting GrimLeague text..." << std::endl;
            ARCExtractor::Extract(inputPath / "mods" / modName / "resources" / "Text_EN.arc", outputPath);
            std::cout << "Extracting GrimLeague items..." << std::endl;
            ARCExtractor::Extract(inputPath / "mods" / modName / "resources" / "Items.arc", outputPath);
        }

        std::cout << "Generating item DB..." << std::endl;
        ItemDBR::BuildItemDB(outputPath, outputPath);
        std::cout << "Generating crafting tables..." << std::endl;
        CraftingDBR::BuildCraftingDB(outputPath, outputPath);
        std::cout << "Done!" << std::endl;
    }
    catch (std::runtime_error& err)
    {
        std::cout << "Failed to extract one or more files: " << err.what() << std::endl;
    }
}