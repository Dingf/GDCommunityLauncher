#ifndef INC_GDCL_HTTP_H
#define INC_GDCL_HTTP_H

#include <string>
#include <memory>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "JSON.h"

namespace asio      = boost::asio;
namespace ssl       = boost::asio::ssl;
using tcp           = asio::ip::tcp;

// These aren't all of the methods/statuses, but only the ones that are relevant to us
enum HTTPMethod
{
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_DELETE,
};

enum HTTPStatus
{
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_NO_CONTENT = 204,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_INTERNAL_ERROR = 500
};

class HTTPResponse
{
    public:
        HTTPStatus GetStatus() const { return _status; }
        const std::string& GetReason() const { return _reason; }
        const std::string& GetBody();
        std::shared_ptr<ssl::stream<tcp::socket>> GetStream() { return _stream; }

        const std::unordered_map<std::string, std::string>& GetHeaders() const { return _headers; }

    private:
        friend class HTTPRequest;

        HTTPResponse(const std::string& response, std::shared_ptr<ssl::stream<tcp::socket>> stream);

        HTTPStatus _status;
        std::string _http;
        std::string _reason;
        std::string _body;
        std::unordered_map<std::string, std::string> _headers;
        std::shared_ptr<ssl::stream<tcp::socket>> _stream;
};

class HTTPRequest
{
    public:
        HTTPRequest(HTTPMethod method, const std::string& target);

        void AddHeader(const std::string& name, const std::string& value);

        void SetBody(const std::string& body);
        void SetBody(const json& body);

        HTTPResponse Send(const std::string& host, const std::string& port);

    private:
        std::string _request;
        std::string _body;
        std::unordered_map<std::string, std::string> _headers;

        asio::io_context& _ioc;
        ssl::context&     _ssl;
        std::shared_ptr<ssl::stream<tcp::socket>> _stream;

};

#endif//INC_GDCL_HTTP_H