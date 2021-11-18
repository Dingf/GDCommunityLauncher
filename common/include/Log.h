#ifndef INC_GDCL_LOG_H
#define INC_GDCL_LOG_H

#include <string>
#include <sstream>
#include <fstream>
#include <time.h>

enum LogLevel
{
    LOG_LEVEL_INFO = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3,
};

class Logger
{
    public:
        static void SetMinimumLogLevel(LogLevel level)
        {
            Logger& logger = GetInstance();
            logger._minLevel = level;
        }

        static void SetLogFilename(const std::string& filename)
        {
            Logger& logger = GetInstance();
            logger._filename = filename;
        }

        static void SetLogFormat(const std::string& format)
        {
            Logger& logger = GetInstance();
            logger._format = format;
        }

        template <typename... Ts>
        static std::string LogMessage(LogLevel level, const std::string& message, Ts... args)
        {
            Logger& logger = GetInstance();

            if (level < logger._minLevel)
                return {};

            std::ofstream out(logger._filename.c_str(), std::ofstream::out | std::ofstream::app);
            if (out.is_open())
            {
                tm currentTime;
                time_t posixTime = time(0);
#if _WIN32
                gmtime_s(&currentTime, &posixTime);
#else
                gmtime_r(&posixTime, &currentTime);
#endif

                std::string year = std::to_string(1900 + currentTime.tm_year);
                std::string month = std::string(((1 + currentTime.tm_mon) < 10), '0') + std::to_string(1 + currentTime.tm_mon);
                std::string day = std::string((currentTime.tm_mday < 10), '0') + std::to_string(currentTime.tm_mday);
                std::string hour = std::string((currentTime.tm_hour < 10), '0') + std::to_string(currentTime.tm_hour);
                std::string minutes = std::string((currentTime.tm_min < 10), '0') + std::to_string(currentTime.tm_min);
                std::string seconds = std::string((currentTime.tm_sec < 10), '0') + std::to_string(currentTime.tm_sec);
                std::string levelString = GetLogLevelString(level);

                std::stringstream ss;
                BuildFormattedMessage(ss, message.c_str(), args...);
                std::string formattedMessage = ss.str();

                std::string output = {};
                for (std::string::const_iterator it = logger._format.begin(); it != logger._format.end(); ++it)
                {
                    char c = *it;
                    if ((c == '$') && ((it + 1) != logger._format.end()))
                    {
                        switch (*(++it))
                        {
                        case 'Y':
                            output += year;
                            break;
                        case 'M':
                            output += month;
                            break;
                        case 'D':
                            output += day;
                            break;
                        case 'h':
                            output += hour;
                            break;
                        case 'm':
                            output += minutes;
                            break;
                        case 's':
                            output += seconds;
                            break;
                        case 'L':
                            output += levelString;
                            break;
                        case 'A':
                            output += formattedMessage;
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        output += c;
                    }
                }
                output += "\n";

                out << output;
                out.close();

                return formattedMessage;
            }

            return {};
        }

    private:
        Logger()
        {
            _minLevel = LOG_LEVEL_INFO;
            _filename = "GDCommunityLauncher.log";
            _format = "[$L] [$Y/$M/$D $h:$m:$s] $A";
        }

        template <typename T, typename... Ts>
        static void BuildFormattedMessage(std::stringstream& result, const char* format, T val0, Ts... vals)
        {
            char c;
            while ((c = *format) != '\0')
            {
                if (c == '%')
                {
                    result << val0;
                    BuildFormattedMessage(result, format + 1, vals...);
                    return;
                }
                result << c;
                format++;
            }
        }

        static void BuildFormattedMessage(std::stringstream& result, const char* format)
        {
            result << format;
        }

        static std::string GetLogLevelString(LogLevel level)
        {
            switch (level)
            {
                case LOG_LEVEL_INFO:
                    return "INFO ";
                    break;
                case LOG_LEVEL_DEBUG:
                    return "DEBUG";
                    break;
                case LOG_LEVEL_WARN:
                    return "WARN ";
                    break;
                case LOG_LEVEL_ERROR:
                    return "ERROR";
                    break;
                default:
                    return {};
                    break;
            }
        }

        static Logger& GetInstance()
        {
            static Logger logger;
            return logger;
        }

        LogLevel _minLevel;
        std::string _filename;
        std::string _format;
};


#endif//INC_GDCL_LOG_H
