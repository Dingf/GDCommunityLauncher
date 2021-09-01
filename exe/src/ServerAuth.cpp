#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <cpprest/http_client.h>
#include "Client.h"
#include "ServerAuth.h"
#include "JSONObject.h"

ServerAuthResult ServerAuthFunction(std::string hostName, std::string username, std::string password, ServerAuthCallback callback)
{
    std::string endpointName = hostName;
    if (endpointName.back() != '/')
        endpointName += '/';
    endpointName += "api/Account/login";

    web::http::client::http_client httpClient(utility::string_t(endpointName.begin(), endpointName.end()));

    web::json::value requestBody;
    requestBody[U("username")] = JSONString(username);
    requestBody[U("password")] = JSONString(password);

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    Client::ServerData data;
    data._name = username;
    data._hostName = hostName;

    // TODO: Replace these with values obtained from the API endpoint
    data._leagueName = "GrimLeague Season 3";
    data._leagueModName = "GrimLeagueS02_HC";

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::value authTokenValue = responseBody[U("access_token")];
            web::json::value refreshTokenValue = responseBody[U("refresh_token")];
            if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
            {
                data._authToken = JSONString(authTokenValue.as_string());
                data._refreshToken = JSONString(refreshTokenValue.as_string());
            }
        }
        else
        {
            if (callback)
                callback(SERVER_AUTH_FAIL);
            return SERVER_AUTH_FAIL;
        }
    }
    catch (...)
    {
        if (callback)
            callback(SERVER_AUTH_TIMEOUT);
        return SERVER_AUTH_TIMEOUT;
    }

    Client& client = Client::GetInstance(data);

    if (callback)
        callback(SERVER_AUTH_OK);
    return SERVER_AUTH_OK;
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