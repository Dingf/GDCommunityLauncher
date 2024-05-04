#include "ClientHandler.h"

const char* HandleGetVersion(void* _this)
{
    Client& client = Client::GetInstance();
    return client.GetVersionInfoText().c_str();
}