#include "ExeClient.h"
#include "ServerAuth.h"
#include "HTTP.h"
#include "Log.h"

ServerAuthResult ServerAuthenticate(ServerAuthCallback callback)
{
    ServerAuthResult result = SERVER_AUTH_TIMEOUT;

    HTTPRequest request(HTTP_POST, "/Account/login");
    request.SetBody({
        { "username", spClient->GetUsername() },
        { "password", spClient->GetPassword() },
    });

    try
    {
        HTTPResponse response = request.Send(spClient->GetHostName(), "443");
        if (response.GetStatus() == 200)
        {
            json responseJSON = json::parse(response.GetBody());
            json& accessToken = responseJSON.at("access_token");
            json& refreshToken = responseJSON.at("refresh_token");

            // If the credentials are not correct, these fields will be null so check them first
            if (accessToken.is_null() || refreshToken.is_null())
                throw std::runtime_error("Invalid username or password");

            spClient->SetAuthToken(accessToken.get<std::string>());
            spClient->SetRefreshToken(refreshToken.get<std::string>());

            if (responseJSON.contains("role"))
                spClient->SetRole(responseJSON.at("role").get<std::string>());

            result = SERVER_AUTH_OK;
        }
        else
        {
            throw std::runtime_error("Server responded with status code " + response.GetStatus());
        }

    }
    catch (std::exception& ex)
    {
        Logger::LogMessage(LOG_LEVEL_WARN, "Failed to login to server: %", ex.what());
        result = SERVER_AUTH_INVALID_LOGIN;
    }

    if (callback)
        callback(result);

    return result;
}