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
        static void OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded);

        // TODO: Load the muted list when the main player is set
};

#endif//INC_GDCL_DLL_SERVER_COORDINATOR_H