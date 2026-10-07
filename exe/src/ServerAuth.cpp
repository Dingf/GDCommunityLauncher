#include "ExeClient.h"
#include "ServerAuth.h"
#include "HTTP.h"
#include "Log.h"

bool GetChatAPI()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Admin/chat-url");
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                spClient->SetChatURL(response.GetBody());
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve chat API: %", ex.what());
    }
    return false;
}

bool GetSeasonName()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Season/latest/season-name?branch=" + spClient->GetBranchName());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                spClient->SetSeasonName(response.GetBody());
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season name: %", ex.what());
    }
    return false;
}

bool GetSeasonData()
{
    HTTPRequest request(HTTP_METHOD_GET, "/Season/latest?branch=" + spClient->GetBranchName());
    request.AddHeader("Authorization", "Bearer " + spClient->GetAuthToken());

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        switch (response.GetStatus())
        {
            case 200:
            {
                json responseJSON = json::parse(response.GetBody());
                for (const json& season : responseJSON)
                {
                    SeasonInfo seasonInfo;
                    seasonInfo._seasonID = season.at("seasonId").get<uint32_t>();
                    seasonInfo._seasonType = season.at("seasonTypeId").get<SeasonType>();
                    seasonInfo._modName = season.at("modName").get<std::string>();
                    seasonInfo._displayName = season.at("displayName").get<std::string>();
                    seasonInfo._participationToken = season.at("participationTag").get<std::string>();

                    // Set the participation token to lower case for standardization
                    for (char& c : seasonInfo._participationToken)
                        c = std::tolower(c);

                    spClient->AddSeason(seasonInfo);
                }
                return true;
            }
            default:
                throw std::runtime_error("Server responded with status code " + std::to_string(response.GetStatus()));
        }
    }
    catch (const std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to retrieve season data: %", ex.what());
    }
    return false;
}

ServerAuthResult ServerAuthenticate(ServerAuthCallback callback)
{
    ServerAuthResult result = SERVER_AUTH_TIMEOUT;
    std::string username = spClient->GetUsername();
    std::string password = spClient->GetPassword();

    if ((username.empty()) || (password.empty()))
    {
        result = SERVER_AUTH_INVALID_LOGIN;
    }
    else
    {
        HTTPRequest request(HTTP_METHOD_POST, "/Account/login");
        request.SetBody({
            { "username", username },
            { "password", password },
        });

        try
        {
            HTTPResponse response = request.Send(spClient->GetHostName(), "443");
            if (response.GetStatus() == 200)
            {
                json responseJSON = json::parse(response.GetBody());
                json& accessToken = responseJSON.at("access_token");
                json& refreshToken = responseJSON.at("refresh_token");

                spClient->SetAuthToken(accessToken.get<std::string>());
                spClient->SetRefreshToken(refreshToken.get<std::string>());

                if (!GetChatAPI())
                {
                    result = SERVER_AUTH_OTHER_ERROR;
                    throw std::runtime_error("Could not retrieve chat API information from the server.");
                }

                if (!GetSeasonName())
                {
                    result = SERVER_AUTH_OTHER_ERROR;
                    throw std::runtime_error("Could not retrieve mod name from the server.");
                }

                if (!GetSeasonData())
                {
                    result = SERVER_AUTH_OTHER_ERROR;
                    throw std::runtime_error("Could not retrieve season information from the server.");
                }

                result = (spClient->HasSeasons()) ? SERVER_AUTH_OK : SERVER_AUTH_NO_ACTIVE_SEASON;
            }
            else if (response.GetStatus() == 400)
            {
                result = SERVER_AUTH_INVALID_LOGIN;
                throw std::runtime_error("Invalid username or password");
            }
            else
            {
                result = SERVER_AUTH_OTHER_ERROR;
                throw std::runtime_error("Server responded with status code " + response.GetStatus());
            }
        }
        catch (std::exception& ex)
        {
            Logger::LogMessage(LOG_LEVEL_WARN, "Failed to login to server: %", ex.what());
        }
    }

    if (callback)
        callback(result);

    return result;
}