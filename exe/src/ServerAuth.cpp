#include <string>
#include <filesystem>
#include <cpprest/http_client.h>
#include "ServerAuth.h"
#include "Date.h"
#include "JSONObject.h"
#include "URI.h"
#include "Version.h"
#include "Log.h"

std::string GetLauncherVersion(const std::string& hostName)
{
    URI endpoint = URI(hostName) / "api" / "File" / "launcher";
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
            web::json::value version = responseBody[U("version")];

            std::string versionString = JSONString(version.serialize());
            return versionString;
        }
        default:
        {
            throw std::runtime_error("Server responded with status code " + std::to_string(response.status_code()));
        }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve launcher version: %", ex.what());
    }
    return {};
}

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
                std::string startDate = JSONString(it->at(U("startDate")).serialize());
                std::string endDate = JSONString(it->at(U("endDate")).serialize());

                std::time_t startDateTime = Date(startDate);
                std::time_t endDateTime = Date(endDate);
                std::time_t currentDateTime = Date();

                if ((data._role == "admin") || (data._role == "tester") || ((currentDateTime >= startDateTime) && (currentDateTime <= endDateTime)))
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

                    for (char& c : participationToken)
                        c = std::tolower(c);

                    seasonInfo._modName = modName;
                    seasonInfo._displayName = displayName;
                    seasonInfo._participationToken = participationToken;

                    data._seasons.push_back(seasonInfo);
                }
            }
            return (data._seasons.size() > 0);
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

ServerAuthResult ServerAuthenticate(ClientData& data, const std::string& password, ServerAuthCallback callback)
{
    URI endpoint = URI(data._hostName) / "api" / "Account" / "login";
    web::http::client::http_client httpClient((utility::string_t)endpoint);

    web::json::value requestBody;
    requestBody[U("username")] = JSONString(data._username);
    requestBody[U("password")] = JSONString(password);

    web::http::http_request request(web::http::methods::POST);
    request.set_body(requestBody);

    try
    {
        web::http::http_response response = httpClient.request(request).get();
        if (response.status_code() == web::http::status_codes::OK)
        {
            web::json::value responseBody = response.extract_json().get();
            web::json::value authTokenValue = responseBody[U("access_token")];
            web::json::value refreshTokenValue = responseBody[U("refresh_token")];
            web::json::value roleValue = responseBody[U("role")];
            if ((!authTokenValue.is_null()) && (!refreshTokenValue.is_null()))
            {
                data._authToken = JSONString(authTokenValue.serialize());
                data._refreshToken = JSONString(refreshTokenValue.serialize());
            }
            if (!roleValue.is_null())
            {
                data._role = JSONString(roleValue.serialize());
            }
        }
        else
        {
            if (callback)
                callback(SERVER_AUTH_INVALID_LOGIN, data);
            return SERVER_AUTH_INVALID_LOGIN;
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to authenticate credentials: %", ex.what());
        if (callback)
            callback(SERVER_AUTH_TIMEOUT, data);
        return SERVER_AUTH_TIMEOUT;
    }

    if (!GetSeasonData(data._hostName, data))
    {
        if (callback)
            callback(SERVER_AUTH_INVALID_SEASONS, data);
        return SERVER_AUTH_INVALID_SEASONS;
    }

    if (GetLauncherVersion(data._hostName) != std::string(GDCL_VERSION))
        data._updateFlag = true;

    if (callback)
        callback(SERVER_AUTH_OK, data);

    return SERVER_AUTH_OK;
}