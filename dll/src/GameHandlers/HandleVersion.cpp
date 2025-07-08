#include "GameHandler.h"
#include "Version.h"

std::string versionInfoText = {};

void BuildVersionInfoText()
{
    std::string message;
    message = EngineAPI::GetVersionString();
    message += "\n{^F}GDCL v";
    message += GDCL_VERSION;
    message += " (";
    if (IsOfflineMode())
        message += "Offline Mode";
    else
        message += GetUsername();
    message += ")";
    versionInfoText = message;
}

const char* HandleGetVersion(void* _this)
{
    if (versionInfoText.empty())
        BuildVersionInfoText();

    return versionInfoText.c_str();
}