#ifndef INC_GDCL_DLL_SERVER_COORDINATOR_H
#define INC_GDCL_DLL_SERVER_COORDINATOR_H

#include <string>

class ServerCoordinator
{
    private:
        ServerCoordinator();
        ServerCoordinator(ServerCoordinator&) = delete;
        void operator=(const ServerCoordinator&) = delete;

        friend bool InitializeModules();

        static ServerCoordinator* GetInstance();

        static void OnShutdownEvent();
        static void OnDirectReadEvent(std::string filename, void** data, size_t* size);
        static void OnDirectWriteEvent(std::string filename, void* data, size_t size);
        static void OnAddSaveJobEvent(std::string filename, void* data, size_t size);
        static void OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded);
        static void OnWorldPreUnloadEvent();
        static void OnSetMainPlayerEvent(void* player);
        static void OnTransferPostLoadEvent();
        static void OnTransferPreSaveEvent();
        static void OnTransferPostSaveEvent();
        static void OnDeleteFileEvent(const char* filename);
        static void OnBestowTokenEvent(std::string token);
};

#endif//INC_GDCL_DLL_SERVER_COORDINATOR_H