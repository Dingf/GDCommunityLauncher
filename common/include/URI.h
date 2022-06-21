#ifndef INC_GDCL_URL_H
#define INC_GDCL_URL_H

#include <string>
#include <cpprest/uri.h>
#include "Log.h"

class URI
{
    public:
        URI() {}
        URI(std::string str) { *this = str; }
        URI(std::wstring str) { *this = str; }
        URI(const char* str) { *this = str; }

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
            return utility::string_t(_data.begin(), _data.end());
        }

        operator std::string() const
        {
            return _data;
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
};

#endif//INC_GDCL_URL_H