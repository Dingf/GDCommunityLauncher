#include "ServerAuth.h"
#include "Client.h"
#include "Log.h"

ServerAuthResult ServerAuthenticate(ServerAuthCallback callback)
{
    ServerAuthResult result = SERVER_AUTH_TIMEOUT;

    Client& client = Client::GetInstance();
    if (Connection* connection = client.GetConnection())
    {
        if (!connection->Connect())
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not connect to the server.");
            return SERVER_AUTH_TIMEOUT;
        }

        if (!connection->Invoke("Login", client.GetUsername(), client.GetPassword()))
        {
            Logger::LogMessage(LOG_LEVEL_ERROR, "Could not authenticate user credentials.");
        }
        else
        {
            // The auth token and refresh token should be populated by this point
            // If they weren't, then that means the login failed (most likely due to invalid credentials)
            if ((client.GetAuthToken().empty()) || (client.GetRefreshToken().empty()))
            {
                result = SERVER_AUTH_INVALID_LOGIN;
            }
            else
            {
                result = SERVER_AUTH_OK;
            }
        }
    }

    if (callback)
        callback(result);

    return result;
}