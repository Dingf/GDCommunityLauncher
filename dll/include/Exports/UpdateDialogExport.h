#ifndef INC_GDCL_DLL_UPDATE_DIALOG_EXPORT_H
#define INC_GDCL_DLL_UPDATE_DIALOG_EXPORT_H

#include <string>
#include <filesystem>

typedef void(*DownloadValueCallback)(size_t);

__declspec(dllexport) std::string GetSeasonModName(const std::string& hostName, const std::string& authToken);
__declspec(dllexport) bool GetDownloadList(const std::string& hostName, const std::string& modName, const std::string& authToken, std::unordered_map<std::wstring, std::string>& updateList);
__declspec(dllexport) bool DownloadFile(const std::filesystem::path& filenamePath, const std::string& downloadURL, DownloadValueCallback totalSizeCallback, DownloadValueCallback downloadSizeCallback);
__declspec(dllexport) bool VerifyBaseGameFiles(const std::string& hostName, const std::string& authToken, std::string& expectedVersion);

#endif//INC_GDCL_DLL_UPDATE_DIALOG_EXPORT_H