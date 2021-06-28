#ifndef INC_GDCL_EXE_CONFIGURATION_H
#define INC_GDCL_EXE_CONFIGURATION_H

#include <map>
#include <string>
#include <filesystem>
#include "Value.h"

class Configuration
{
    public:
        Configuration() {}
        Configuration(const std::filesystem::path& path) { Load(path); }

        bool Load(const std::filesystem::path& path);

        bool Save(const std::filesystem::path& path);

        Value* GetValue(const std::string& section, const std::string& name)
        {
            ConfigKey key({ section, name });
            auto it = _configValues.find(key);
            if (it == _configValues.end())
                return nullptr;
            else
                return &(it->second);
        }

        template <class T>
        void SetValue(const std::string& section, const std::string& name, T val)
        {
            ConfigKey key({ section, name });
            auto it = _configValues.find(key);
            if (it != _configValues.end())
                _configValues.erase(it);
            _configValues.emplace(key, val);
        }

    private:
        typedef std::pair<std::string, std::string> ConfigKey;

        void InsertValue(const std::string& section, const std::string& key, const std::string& value);

        std::map<ConfigKey, Value> _configValues;
};

#endif//INC_GDCL_EXE_CONFIGURATION_H