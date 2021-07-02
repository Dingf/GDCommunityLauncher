#include <map>
#include <thread>
#include <future>
#include <chrono>
#include "Client.h"
#include "ServerAuth.h"

const std::map<std::string,std::string> fakeServerCredentials =
{
    { "Mr Monday", "Testing123" },
    { "Mr Tuesday", "Someotherpassword" },
    { "Mr Wednesday", "Yet Another Password?" }
};

ServerAuthResult FakeServerAuth(std::string hostName, std::string username, std::string password, ServerAuthCallback callback)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));

    if ((fakeServerCredentials.count(username) > 0) && (fakeServerCredentials.at(username) == password))
    {
        std::string authToken("TODO: This should actually be a randomly generated string");

        Client& client = Client::GetInstance(username, authToken, hostName);
        if (callback)
            callback(SERVER_AUTH_OK);
        return SERVER_AUTH_OK;
    }
    else
    {
        if (callback)
            callback(SERVER_AUTH_FAIL);
        return SERVER_AUTH_FAIL;
    }
}

void ServerAuth::ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password, ServerAuthCallback callback)
{
    std::thread t(&FakeServerAuth, hostName, username, password, callback);
    t.detach();
}

ServerAuthResult ServerAuth::ValidateCredentials(const std::string& hostName, const std::string& username, const std::string& password)
{
    std::future<ServerAuthResult> future = std::async(&FakeServerAuth, hostName, username, password, nullptr);
    return future.get();
}