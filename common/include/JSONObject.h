#ifndef INC_GDCL_JSON_OBJECT_H
#define INC_GDCL_JSON_OBJECT_H

#include <string>
#include <cpprest/json.h>

class JSONObject
{
    public:
        virtual web::json::value ToJSON() = 0;
};

class JSONString
{
    public:
        JSONString(const std::string& s) : _s(s) {}
        JSONString(const utility::string_t& s) : _s(utility::conversions::to_utf8string(s)) {}

        operator web::json::value() const
        {
            return web::json::value::string(utility::conversions::to_utf16string(_s));
        }

        operator std::string() const { return _s; }

    private:
        const std::string& _s;
};

#endif//INC_GDCL_JSON_OBJECT_H