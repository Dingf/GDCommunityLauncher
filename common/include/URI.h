#ifndef INC_GDCL_URL_H
#define INC_GDCL_URL_H

#include <string>
#include <unordered_map>
#include <cpprest/uri.h>
#include "JSONObject.h"

class URI
{
    public:
        URI() {}
        URI(std::string str) { _data = str; }
        URI(std::wstring str) { *this = str; }
        URI(const char* str) { _data = str; }
        URI(JSONString& str) { _data = (std::string)str; }

        URI& operator=(const URI& uri)
        {
            _data = uri._data;
            return *this;
        }

        template <class T>
        URI& operator=(std::basic_string<T> str)
        {
            _data.clear();
            Append(str, false);
            return *this;
        }

        URI& operator=(const char* str)
        {
            _data = str;
            return *this;
        }

        template <class T>
        URI& operator/=(T&& right)
        {
            const URI& str(right);
            if (_data.back() != '/')
                _data.push_back('/');
            Append(str._data);
            return *this;
        }

        template <class T>
        URI& operator+=(T&& right)
        {
            const URI& str(right);
            Append(str._data);
            return *this;
        }

        template <class T>
        friend URI operator/(const URI& left, T&& right)
        {
            return URI(left) /= URI(right);
        }

        template <class T>
        friend URI operator+(const URI& left, T&& right)
        {
            return URI(left) += URI(right);
        }

        operator utility::string_t() const
        {
            std::string result = operator std::string();
            return utility::string_t(result.begin(), result.end());
        }

        operator std::string() const
        {
            std::string result = _data;
            if (_params.size() > 0)
            {
                size_t count = 0;
                for (auto pair : _params)
                {
                    result += (count++ == 0) ? "?" : "&";
                    result += pair.first;
                    result += "=";
                    result += pair.second;
                }
            }
            return result;
        }

        bool empty() const { return _data.empty(); }
        size_t size() const { return _data.size(); }

        template <class T>
        void AddParam(const std::string& key, const T& value)
        {
            std::string result = std::to_string(value);
            if (!result.empty())
                _params[key] = result;
        }

        template <>
        void AddParam(const std::string& key, const std::string& value)
        {
            if (!value.empty())
                _params[key] = value;
        }

        template <>
        void AddParam(const std::string& key, const bool& value)
        {
            std::string result = (value) ? "true" : "false";
            if (!result.empty())
                _params[key] = result;
        }

        template <class T>
        void Append(const std::basic_string<T>& str, bool encode = true)
        {
            if (encode)
            {
                std::stringstream result;
                result << std::hex;
                for (size_t i = 0; i < str.size(); ++i)
                {
                    T c = str[i];
                    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                    {
                        result << (char)c;
                        continue;
                    }
                    result << "%" << (uint32_t)c;
                }
                _data += result.str();
            }
            else
            {
                for (size_t i = 0; i < str.size(); ++i)
                {
                    _data.push_back((char)str[i]);
                }
            }
        }

    private:
        std::string _data;
        std::unordered_map<std::string, std::string> _params;
};

#endif//INC_GDCL_URL_H