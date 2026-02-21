#ifndef INC_GDCL_HTTP_REQUEST_H
#define INC_GDCL_HTTP_REQUEST_H

#include <string>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/scoped_ptr.hpp>

namespace asio      = boost::asio;
namespace ssl       = boost::asio::ssl;
using tcp           = asio::ip::tcp;

class HTTPRequest
{
    public:
        typedef ssl::stream<tcp::socket> SSLStream;

        HTTPRequest(asio::io_context& ioc, ssl::context& ssl) : _ioc(ioc), _ssl(ssl)
        {
            _ss.reset(new SSLStream(_ioc, _ssl));
        }

        std::string Send(std::string host, std::string port, std::string request)
        {
            tcp::resolver resolver(_ioc);
            asio::connect(_ss->lowest_layer(), resolver.resolve(host, port));
            _ss->handshake(ssl::stream_base::client);

            asio::write(*_ss, asio::buffer(request));

            std::string response;
            boost::system::error_code ec;

            do
            {
                char buffer[1024];
                size_t bytesRead = _ss->read_some(asio::buffer(buffer), ec);
                if (!ec)
                    response.append(buffer, bytesRead);
            }
            while (!ec);

            return response;
        }

    private:
        asio::io_context& _ioc;
        ssl::context&     _ssl;
        boost::scoped_ptr<SSLStream> _ss;

};

#endif//INC_GDCL_HTTP_REQUEST_H