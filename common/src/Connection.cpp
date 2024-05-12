#include <signalrclient/hub_connection_builder.h>
#include "Connection.h"

class ConnectionLogger : public signalr::log_writer
{
    // Inherited via log_writer
    virtual void __cdecl write(const std::string& entry) override
    {
        Logger::LogMessage(LOG_LEVEL_INFO, entry);
    }
};

Connection::Connection(URI endpoint)
{
    _connection = std::make_unique<signalr::hub_connection>(
        signalr::hub_connection_builder::create(endpoint)
        .skip_negotiation()
        .with_logging(std::make_shared<ConnectionLogger>(), signalr::trace_level::verbose)
        .build());
}

Connection::~Connection()
{
    Disconnect();
}

bool Connection::IsConnected() const
{
    return _connection->get_connection_state() == signalr::connection_state::connected;
}

signalr::connection_state Connection::GetConnectionState() const
{
    return _connection->get_connection_state();
}

bool Connection::Connect()
{
    if (_connection)
    {
        std::promise<bool> task;
        _connection->start([&task](std::exception_ptr exp)
        {
            try
            {
                if (exp)
                    std::rethrow_exception(exp);
                task.set_value(true);
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to start connection: %", ex.what());
                task.set_value(false);
            }
        });
        return task.get_future().get();
    }
    return false;
}

bool Connection::Disconnect()
{
    if (_connection)
    {
        std::promise<bool> task;
        _connection->stop([&task](std::exception_ptr exp)
        {
            try
            {
                if (exp)
                    std::rethrow_exception(exp);
                task.set_value(true);
            }
            catch (const std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to shutdown connection: %", ex.what());
                task.set_value(false);
            }
        });
        return task.get_future().get();
    }
    return false;
}