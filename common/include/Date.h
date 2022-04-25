#ifndef INC_GDCL_DATE_H
#define INC_GDCL_DATE_H

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <regex>

class Date
{
public:
    Date() { time(&_time); }
    Date(std::time_t val) { _time = val; }
    Date(std::string& val)
    {
        std::smatch match;
        const std::regex dateRegex("^(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})$");
        if (std::regex_match(val, match, dateRegex))
        {
            std::tm date = { 0 };
            date.tm_year = std::stoi(match.str(1)) - 1900;
            date.tm_mon = std::stoi(match.str(2)) - 1;
            date.tm_mday = std::stoi(match.str(3));
            date.tm_hour = std::stoi(match.str(4));
            date.tm_min = std::stoi(match.str(5));
            date.tm_sec = std::stoi(match.str(6));

#if _WIN32
            _time = _mkgmtime(&date);
#else
            _time = timegm(&date);
#endif
        }
        else
        {
            throw std::runtime_error("Could not parse date string \"" + val + "\"");
        }
    }

    operator std::time_t() const
    {
        return _time;
    }

    operator std::string() const
    {
        std::tm date;
        std::stringstream ss;

        gmtime_s(&date, &_time);

        ss << date.tm_year + 1900 << "-";
        ss << std::setfill('0') << std::setw(2) << date.tm_mon + 1 << "-";
        ss << std::setfill('0') << std::setw(2) << date.tm_mday << "T";
        ss << std::setfill('0') << std::setw(2) << date.tm_hour << ":";
        ss << std::setfill('0') << std::setw(2) << date.tm_min << ":";
        ss << std::setfill('0') << std::setw(2) << date.tm_sec;
        return ss.str();
    }

private:
    std::time_t _time;
};


#endif//INC_GDCL_DATE_H