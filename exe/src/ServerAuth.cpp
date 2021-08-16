#include <map>
#include <thread>
#include <future>
#include <chrono>
#include "Client.h"
#include "ServerAuth.h"
#include "JSONObject.h"
#include <cpprest/filestream.h>
#include <cpprest/http_client.h>

ServerAuthResult ServerAuthFunction(std::string hostName, std::string username, std::string password, ServerAuthCallback callback)
{
    if (hostName.back() != '/')
        hostName += '/';
    hostName += "api/Account/login";

    web::http::client::http_client client(utility::string_t(hostName.begin(), hostName.end()));

    web::json::value requestBody;
    requestBody[U("username")] = JSONString(username);
    requestBody[U("password")] = JSONString(password);

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    try
    {
        web::http::http_response response = client.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::value authTokenValue = responseBody[U("access_token")];
            if ((!authTokenValue.is_null()) && (authTokenValue.is_string()))
            {
                JSONString authTokenString = authTokenValue.as_string();
                Client& client = Client::GetInstance(username, authTokenString, hostName);

                if (callback)
                    callback(SERVER_AUTH_OK);
                return SERVER_AUTH_OK;
            }
        }
    }
    catch (...)
    {
        if (callback)
            callback(SERVER_AUTH_TIMEOUT);
        return SERVER_AUTH_TIMEOUT;
    }

    if (callback)
        callback(SERVER_AUTH_FAIL);
    return SERVER_AUTH_FAIL;
}

void ServerAuth::ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password, ServerAuthCallback callback)
{
    std::thread t(&ServerAuthFunction, hostName, username, password, callback);
    t.detach();
}

ServerAuthResult ServerAuth::ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password)
{
    std::future<ServerAuthResult> future = std::async(&ServerAuthFunction, hostName, username, password, nullptr);
    return future.get();
}