#ifndef INC_GDCL_CONNECTION_H
#define INC_GDCL_CONNECTION_H

#include <string>
#include <vector>
#include <memory>
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <signalrclient/hub_connection.h>
#include <signalrclient/signalr_value.h>
#include "URI.h"
#include "StringConvert.h"
#include "Log.h"

class Connection
{
    public:

        typedef void (*ConnectionHandler)(const signalr::value&, const std::vector<void*>);

        Connection(URI endpoint);
        ~Connection();

        bool IsConnected() const;
        signalr::connection_state GetConnectionState() const;

        bool Connect();
        bool Disconnect();
        
        template <typename... Ts>
        void InvokeAsync(std::string function, Ts... vals)
        {
            std::vector<signalr::value> args;
            BuildFunctionArgs(args, vals...);
            _connection->invoke(function, args, [function](const signalr::value& value, std::exception_ptr exp)
            {
                try
                {
                    if (exp)
                        std::rethrow_exception(exp);
                }
                catch (const std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to invoke function \"%\": %", function, ex.what());
                }
            });
        }

        template <typename... Ts>
        bool Invoke(std::string function, std::vector<void*> sharedArgs, Ts... vals)
        {
            _sharedArgs[function] = sharedArgs;
            return Invoke(function, vals...);
        }

        template <typename... Ts>
        bool Invoke(std::string function, Ts... vals)
        {
            std::promise<bool> task;
            std::vector<signalr::value> args;
            BuildFunctionArgs(args, vals...);
            _connection->invoke(function, args, [&task, function](const signalr::value& value, std::exception_ptr exp)
            {
                try
                {
                    if (exp)
                        std::rethrow_exception(exp);
                    task.set_value(true);
                }
                catch (const std::exception& ex)
                {
                    Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to invoke function \"%\": %", function, ex.what());
                    task.set_value(false);
                }
            });
            return task.get_future().get();
        }

        template <typename T>
        void Register(std::string function, T handler)
        {
            _connection->on(function, handler);
        }

        void Register(std::string function, ConnectionHandler handler)
        {
            if (_handlers.count(function) == 0)
            {
                _connection->on(function, [this, function](const signalr::value& value)
                {
                    for (ConnectionHandler callback : _handlers[function])
                    {
                        callback(value, _sharedArgs[function]);
                    }
                });
            }
            _handlers[function].insert(handler);
        }
        
    protected:
        void BuildFunctionArgs(std::vector<signalr::value>& args) {}

        template<typename T>
        void BuildFunctionArgs(std::vector<signalr::value>& args, T val0)
        {
            args.push_back(val0);
        }

        template <typename T, typename... Ts>
        void BuildFunctionArgs(std::vector<signalr::value>& args, T val0, Ts... vals)
        {
            args.push_back(val0);
            BuildFunctionArgs(args, vals...);
        }

        // uint32_t specialization because SignalR doesn't have support for integers...
        template <typename... Ts>
        void BuildFunctionArgs(std::vector<signalr::value>& args, uint32_t val0, Ts... vals)
        {
            args.push_back((double)val0);
            BuildFunctionArgs(args, vals...);
        }

        // wstring specialization
        template <typename... Ts>
        void BuildFunctionArgs(std::vector<signalr::value>& args, std::wstring val0, Ts... vals)
        {
            args.push_back(WideToChar(val0));
            BuildFunctionArgs(args, vals...);
        }

        std::unordered_map<std::string, std::vector<void*>> _sharedArgs;
        std::unordered_map<std::string, std::unordered_set<ConnectionHandler>> _handlers;
        std::unique_ptr<signalr::hub_connection> _connection;
};

#endif//INC_GDCL_CONNECTION_H