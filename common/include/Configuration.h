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

        const Value* GetValue(const std::string& section, const std::string& name);

        template <class T>
        void SetValue(const std::string& section, const std::string& name, T val)
        {
            ConfigKey key({ section, name });
            auto it = _configValues.find(key);
            if (it != _configValues.end())
                _configValues.erase(it);
            _configValues.insert(std::make_pair(key, std::make_unique<Value>(val)));
        }

        template <>
        void SetValue(const std::string& section, const std::string& name, const std::string& val)
        {
            ConfigKey key({ section, name });
            auto it = _configValues.find(key);
            if (it != _configValues.end())
                _configValues.erase(it);
            _configValues.insert(std::make_pair(key, Value::Parse(val)));
        }

        void DeleteValue(const std::string& section, const std::string& name)
        {
            ConfigKey key({ section, name });
            auto it = _configValues.find(key);
            if (it != _configValues.end())
                _configValues.erase(it);
        }

    private:
        typedef std::pair<std::string, std::string> ConfigKey;

        std::map<ConfigKey, std::unique_ptr<Value>> _configValues;
};

#endif//INC_GDCL_EXE_CONFIGURATION_H