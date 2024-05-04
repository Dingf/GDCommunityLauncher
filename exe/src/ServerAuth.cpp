#include <string>
#include <filesystem>
#include <cpprest/http_client.h>
#include "ServerAuth.h"
#include "Client.h"
#include "Date.h"
#include "JSONObject.h"
#include "URI.h"
#include "Log.h"

ServerAuthResult ServerAuthenticate(ServerAuthCallback callback)
{
    Client& client = Client::GetInstance();
    URI gameURL = client.GetServerGameURL();
    URI endpoint = gameURL / "Account" / "login";
    web::http::client::http_client httpClient((utility::string_t)endpoint);

    web::json::value requestBody;
    requestBody[U("username")] = JSONString(client._username);
    requestBody[U("password")] = JSONString(client._password);

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
                client._authToken = JSONString(authTokenValue.serialize());
                client._refreshToken = JSONString(refreshTokenValue.serialize());
            }
            if (!roleValue.is_null())
            {
                client._role = JSONString(roleValue.serialize());
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

    if (callback)
        callback(SERVER_AUTH_OK);

    return SERVER_AUTH_OK;
}