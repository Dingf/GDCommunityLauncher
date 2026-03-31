#ifndef INC_GDCL_WEBSOCKET_H
#define INC_GDCL_WEBSOCKET_H

#include <atomic>
#include <string>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/scoped_ptr.hpp>
#include "ThreadSafeQueue.h"
#include "Log.h"

namespace asio      = boost::asio;
namespace beast     = boost::beast;
namespace websocket = beast::websocket;
namespace ssl       = boost::asio::ssl;
using tcp           = asio::ip::tcp;

template <class T, typename U>
class Websocket
{
    public:
        typedef beast::websocket::stream<ssl::stream<tcp::socket>> WebsocketStream;

        Websocket(asio::io_context& ioc, ssl::context& ssl, T& handler) : _ioc(ioc), _ssl(ssl), _timer(ioc), _handler(handler), _connected(false)
        {
            _ws.reset(new WebsocketStream(_ioc, _ssl));
            // TODO: Make these configurable?
            _ws->write_buffer_bytes(262144);
            _ws->set_option(websocket::stream_base::timeout({std::chrono::seconds(30), websocket::stream_base::none(), false}));
        }

        bool IsConnected() const { return _connected; }

        bool Connect(std::string host, std::string port, std::string target, std::string authToken = {})
        {
            try
            {
                _host = host;
                _port = port;
                _target = target;
                _authToken = authToken;

                tcp::resolver resolver(_ioc);
                tcp::endpoint endpoint = asio::connect(beast::get_lowest_layer(*_ws), resolver.resolve(host, port));

                if (!SSL_set_tlsext_host_name(_ws->next_layer().native_handle(), host.c_str()))
                    throw std::runtime_error("Failed to set SNI hostname");

                if (!authToken.empty())
                {
                    _ws->set_option(websocket::stream_base::decorator([authToken](websocket::request_type& request)
                    {
                        request.set(beast::http::field::authorization, "Bearer " + authToken);
                    }));
                }

                _ws->next_layer().handshake(ssl::stream_base::client);
                _ws->handshake(host, target);
                _connected = true;

                Read();
                Ping();
                return true;
            }
            catch (std::exception& ex)
            {
                Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to connect to %:%: %", host, port, ex.what());
                return false;
            }
        };

        void Disconnect()
        {
            if (_connected)
            {
                auto& socket = beast::get_lowest_layer(*_ws);
                socket.cancel();
                socket.shutdown(tcp::socket::shutdown_both);
                _ws->async_close(websocket::close_code::normal, [&](const beast::error_code& ec)
                {
                    _ws.reset(new WebsocketStream(_ioc, _ssl));
                    _connected = false;
                });
            }
        }

        template <typename... Ts>
        U Send(Ts... args)
        {
            std::string message;
            U result = _handler.OnWrite(message, args...);

            if (!message.empty())
            {
                asio::post(_ws->get_executor(), [this, message]()
                {
                    _messageQueue.emplace(std::move(message));
                    if (_messageQueue.size() == 1)
                        Write();
                });
            }
            return result;
        }

    private:
        std::atomic_bool _connected; // The current state of the connection
        std::string _host;           // The last used hostname
        std::string _port;           // The last used port number
        std::string _target;         // The last used target
        std::string _authToken;      // The last used auth token

        asio::io_context&  _ioc;
        ssl::context&      _ssl;
        asio::steady_timer _timer;
        beast::flat_buffer _buffer;
        boost::scoped_ptr<WebsocketStream> _ws;

        ThreadSafeQueue<std::string> _messageQueue;

        T& _handler;

        void Read()
        {
            _ws->async_read(_buffer, [this](const beast::error_code& ec, size_t n)
            {
                if (!ec)
                {
                    std::string message = beast::buffers_to_string(_buffer.data()).substr(0, n);
                    _buffer.consume(n);
                    _handler.OnRead(message);
                    Read();
                }
                else
                {
                    Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to read data from websocket: %", ec.what());
                    Disconnect();
                }
            });
        }

        void Write()
        {
            std::string message;
            if (_messageQueue.front(message))
            {
                _ws->async_write(asio::buffer(message), [this](const beast::error_code& ec, size_t n)
                {
                    if (!ec)
                    {
                        _messageQueue.pop();
                        Write();
                    }
                    else
                    {
                        Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to write data to websocket: %", ec.what());
                        Disconnect();
                    }
                });
            }
        }

        void Ping()
        {
            // TODO: Make this configurable?
            _timer.expires_after(std::chrono::milliseconds(1000));
            _timer.async_wait([this](const boost::system::error_code& ec)
            {
                if (!ec)
                {
                    if (_connected)
                    {
                        _ws->async_ping({}, [this](const beast::error_code& ec)
                        {
                            if (ec)
                            {
                                // Reset the connection on error to allow the client to reconnect
                                Disconnect();
                            }
                            Ping();
                        });
                    }
                    else
                    {
                        if (!Connect(_host, _port, _target, _authToken))
                            Ping();
                    } 
                }
                else
                {
                    Logger::LogMessage(LOG_LEVEL_ERROR, "Failed to ping websocket: %", ec.what());
                    Disconnect();
                }
            });
        }
};

#endif//INC_GDCL_WEBSOCKET_H