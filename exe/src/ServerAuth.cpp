#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <cpprest/http_client.h>
#include "Client.h"
#include "ServerAuth.h"
#include "JSONObject.h"
#include "URI.h"
#include "Log.h"

bool GetSeasonData(std::string hostName, ClientData& data)
{
    URI endpoint = URI(hostName) / "api" / "Season" / "latest";
    web::http::client::http_client httpClient((utility::string_t)endpoint);
    web::http::http_request request(web::http::methods::GET);

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        switch (response.status_code())
        {
            case web::http::status_codes::OK:
            {
                web::json::value responseBody = response.extract_json().get();
                web::json::array seasonsList = responseBody.as_array();
                for (auto it = seasonsList.begin(); it != seasonsList.end(); ++it)
                {
                    SeasonInfo seasonInfo;
                    seasonInfo._seasonID = it->at(U("seasonId")).as_integer();
                    seasonInfo._seasonType = it->at(U("seasonTypeId")).as_integer();

                    std::string modName = JSONString(it->at(U("modName")).serialize());
                    std::string displayName = JSONString(it->at(U("displayName")).serialize());
                    std::string participationToken = JSONString(it->at(U("participationTag")).serialize());

                    // Trim quotes from serializing the string
                    if ((modName.front() == '"') && (modName.back() == '"'))
                        modName = std::string(modName.begin() + 1, modName.end() - 1);
                    if ((displayName.front() == '"') && (displayName.back() == '"'))
                        displayName = std::string(displayName.begin() + 1, displayName.end() - 1);
                    if ((participationToken.front() == '"') && (participationToken.back() == '"'))
                        participationToken = std::string(participationToken.begin() + 1, participationToken.end() - 1);

                    seasonInfo._modName = modName;
                    seasonInfo._displayName = displayName;
                    seasonInfo._participationToken = participationToken;

                    data._seasons.push_back(seasonInfo);
                }
                return true;
            }
            default:
            {
                throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season data: %", ex.what());
    }
    return false;
}

ServerAuthResult ServerAuthFunction(std::string hostName, std::string username, std::string password, ServerAuthCallback callback)
{
    URI endpoint = URI(hostName) / "api" / "Account" / "login";
    web::http::client::http_client httpClient((utility::string_t)endpoint);

    web::json::value requestBody;
    requestBody[U("username")] = JSONString(username);
    requestBody[U("password")] = JSONString(password);

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    ClientData data;
    data._username = username;
    data._hostName = hostName;

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
                callback(SERVER_AUTH_INVALID_LOGIN);
            return SERVER_AUTH_INVALID_LOGIN;
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to authenticate credentials: %", ex.what());
        if (callback)
            callback(SERVER_AUTH_TIMEOUT);
        return SERVER_AUTH_TIMEOUT;
    }

    if (!GetSeasonData(hostName, data))
    {
        if (callback)
            callback(SERVER_AUTH_INVALID_SEASONS);
        return SERVER_AUTH_INVALID_SEASONS;
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