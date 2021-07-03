#include "Value.h"

bool ParseInt(const std::string& s, int* val)
{
    if (s.length() == 0)
        return false;

    uint32_t i = 0;
    bool negative = false;

    if ((s[i] == '-') || (s[i] == '+'))
        negative = (s[i++] == '-');

    int result = 0;
    while (i < s.length())
    {
        if ((s[i] >= '0') && (s[i] <= '9'))
        {
            int digit = s[i] - '0';

            // Exceeds int limit, return false
            if ((result > INT_MAX / 10) || ((result == INT_MAX / 10) && (digit > (7 + negative))))
                return false;

            result = (result * 10) + digit;
        }
        else
        {
            return false;
        }
        i++;
    }
    *val = (negative) ? -result : result;
    return true;
}

bool ParseFloat(const std::string& s, float* val)
{
    if (s.length() == 0)
        return false;

    uint32_t i = 0;
    bool negative = false;
    int decimal = 0;

    if ((s[i] == '-') || (s[i] == '+'))
        negative = (s[i++] == '-');

    float result = 0.0f;
    while (i < s.length())
    {
        if ((s[i] >= '0') && (s[i] <= '9'))
        {
            int digit = s[i] - '0';

            if (decimal == 0)
            {
                result = (result * 10) + digit;
            }
            // Round to 6 decimal places
            else if (decimal <= 6)
            {
                if (digit)
                    result += (float)digit / pow(10.0f, (float)decimal);
                decimal++;
            }
        }
        else if ((s[i] == '.') && (decimal == 0))
        {
            decimal = 1;
        }
        else
        {
            return false;
        }
        i++;
    }
    *val = (negative) ? -result : result;
    return true;
}

bool ParseBool(const std::string& s, bool* val)
{
    if (s.length() == 0)
        return false;

    // Convert to lowercase before comparing
    std::string copy(s);
    for (uint32_t i = 0; i < s.length(); ++i)
    {
        if ((copy[i] >= 'A') && (copy[i] <= 'Z'))
            copy[i] += 32;
    }

    if (copy == "true")
    {
        *val = true;
        return true;
    }
    else if (copy == "false")
    {
        *val = false;
        return true;
    }
    return false;
}

Value::Value(const Value& val) : _type(val._type)
{
    switch (val._type)
    {
    case VALUE_TYPE_INT:
        _i = val._i;
        break;
    case VALUE_TYPE_FLOAT:
        _f = val._f;
        break;
    case VALUE_TYPE_BOOL:
        _b = val._b;
        break;
    case VALUE_TYPE_STRING:
        CopyStringData(val._s);
        break;
    }
}

std::unique_ptr<Value> Value::Parse(const std::string& s)
{
    int i;
    float f;
    bool b;

    if (ParseInt(s, &i))
        return std::make_unique<Value>(i);
    else if (ParseFloat(s, &f))
        return std::make_unique<Value>(f);
    else if (ParseBool(s, &b))
        return std::make_unique<Value>(b);
    else
        return std::make_unique<Value>(s.c_str());
}

void Value::DestroyStringData()
{
    if ((_type == VALUE_TYPE_STRING) && (_s != nullptr))
    {
        delete[] _s;
        _s = nullptr;
    }
}

void Value::CopyStringData(const char* val)
{
    if (_type == VALUE_TYPE_STRING)
    {
        DestroyStringData();
        if (val != nullptr)
        {
            _s = new char[256]();
            for (int i = 0; i < 255; ++i)
            {
                _s[i] = val[i];
                if (val[i] == '\0')
                    break;
            }
        }
    }
}