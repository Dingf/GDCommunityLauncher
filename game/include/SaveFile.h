#ifndef INC_GDCL_GAME_SAVE_FILE_H
#define INC_GDCL_GAME_SAVE_FILE_H

#include <filesystem>

class SaveFile
{
    public:
        virtual bool ReadFromFile(const std::filesystem::path& path) = 0;
        virtual void WriteToFile(const std::filesystem::path& path) = 0;

    protected:
        virtual size_t CalculateBufferSize() const = 0;
};

#endif//INC_GDCL_GAME_SAVE_FILE_H