#include <sstream>
#include "HTTP.h"
#include "ContextManager.h"
#include "Log.h"

inline std::string GetMethodName(HTTPMethod method)
{
    switch (method)
    {
        case HTTP_GET:
            return "GET";
        case HTTP_POST:
            return "POST";
        case HTTP_DELETE:
            return "DELETE";
        default:
            return "";
    }
    return {};
}

HTTPResponse::HTTPResponse(const std::string& header, std::shared_ptr<ssl::stream<tcp::socket>> stream)
{
    std::stringstream headerStream(header);

    headerStream >> _http >> _status;
    headerStream.ignore(1);
    std::getline(headerStream, _reason, '\r');
    headerStream.ignore(1);

    for (std::string line; std::getline(headerStream, line, '\r');)
    {
        headerStream.ignore(1);
        size_t split = line.find(':');
        if (split != std::string::npos)
            _headers.emplace(line.substr(0, split), line.substr(split + 2));
    }

    _stream = stream;
}

const std::string& HTTPResponse::GetBody()
{
    if (_stream)
    {
        char buffer[1024];
        boost::system::error_code ec;

        do
        {
            size_t bytesRead = _stream->read_some(asio::buffer(buffer, 1024), ec);

            if (!ec)
                _body.append(buffer, bytesRead);
        }
        while (!ec);

        // Special case for chunked encoding
        auto it = _headers.find("Transfer-Encoding");
        if (it != _headers.end())
        {
            std::string contentType = it->second;
            if (contentType == "chunked")
            {
                size_t size = 0;
                std::string newBody;
                std::stringstream bodyStream(_body);

                do
                {
                    bodyStream >> std::hex >> size;
                    bodyStream.ignore(2);
                    bodyStream.read(buffer, size);
                    bodyStream.ignore(2);
                    newBody.append(buffer, size);
                }
                while (size > 0);

                _body = newBody;
            }
        }
    }
    return _body;
}

HTTPRequest::HTTPRequest(HTTPMethod method, const std::string& target) : _ioc(ContextManager::GetIOContext()), _ssl(ContextManager::GetSSLContext())
{
    _stream = std::make_shared<ssl::stream<tcp::socket>>(_ioc, _ssl);

    _request = GetMethodName(method) + " " + target + " HTTP/1.1\r\n";
    _body = {};
}

void HTTPRequest::AddHeader(const std::string& name, const std::string& value)
{
    _headers.emplace(name, value);
}

void HTTPRequest::SetBody(const std::string& body)
{
    AddHeader("Accept", "*/*");
    AddHeader("Content-Type", "text/plain");
    _body = body;
}

void HTTPRequest::SetBody(const json& body)
{
    AddHeader("Accept", "application/json");
    AddHeader("Content-Type", "application/json");
    _body = body.dump();
}

HTTPResponse HTTPRequest::Send(const std::string& host, const std::string& port)
{
    tcp::resolver resolver(_ioc);
    asio::connect(_stream->lowest_layer(), resolver.resolve(host, port));
    _stream->handshake(ssl::stream_base::client);

    AddHeader("Content-Length", std::to_string(_body.size()));
    AddHeader("Connection", "close");
    AddHeader("Host", host);

    for (const auto& pair: _headers)
    {
        _request += pair.first + ": " + pair.second + "\r\n";
    }
    _request += "\r\n";
    _request += _body;
    _request += "\r\n";

    asio::write(*_stream, asio::buffer(_request));

    char c;
    uint32_t end = 0x00000000;
    std::string header;
    boost::system::error_code ec;

    do
    {
        _stream->read_some(asio::buffer(&c, 1), ec);
        if (!ec)
        {
            header.push_back(c);
            end <<= 8;
            end |= c;
        }
    }
    while (!ec && (end != 0x0D0A0D0A)); // This checks for \r\n\r\n as the last four read characters

    return HTTPResponse(header, _stream);
}