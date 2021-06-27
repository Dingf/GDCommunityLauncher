#include <map>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include "Client.h"
#include "ServerAuth.h"

const std::map<std::string,std::string> fakeServerCredentials =
{
    { "Mr Monday", "Testing123" },
    { "Mr Tuesday", "Someotherpassword" },
    { "Mr Wednesday", "Yet Another Password?" }
};

void FakeServerAuth(const boost::system::error_code&, std::string username, std::string password, ServerAuthCallback callback)
{
    if ((fakeServerCredentials.count(username) > 0) && (fakeServerCredentials.at(username) == password))
    {
        std::string authToken("TODO: This should actually be a randomly generated string");

        Client& client = Client::GetInstance(username, authToken);
        callback(SERVER_AUTH_OK);
    }
    else
    {
        callback(SERVER_AUTH_FAIL);
    }
}

void CreateFakeServerAuthThread(const std::string& username, const std::string& password, ServerAuthCallback callback)
{
    // TODO: This is temporary placeholder code to mimic the server behavior until the API is implemented
    //       Replace me with actual web API calls later
    boost::asio::io_context io;

    boost::asio::deadline_timer timer(io, boost::posix_time::seconds(2));
    timer.async_wait(boost::bind(FakeServerAuth, boost::asio::placeholders::error, username, password, callback));

    io.run();
}

void ServerAuth::ValidateCredentials(const std::string& username, const std::string& password, ServerAuthCallback callback)
{
    boost::thread t(boost::bind(CreateFakeServerAuthThread, username, password, callback));
}