#ifndef INC_GDCL_DLL_SERVER_COORDINATOR_H
#define INC_GDCL_DLL_SERVER_COORDINATOR_H

#include <string>

class ServerCoordinator
{
    public:
        static bool Initialize();

    private:
        ServerCoordinator();
        ServerCoordinator(ServerCoordinator&) = delete;
        void operator=(const ServerCoordinator&) = delete;

        static ServerCoordinator* GetInstance();

        static void OnShutdownEvent();
        static void OnDirectReadEvent(std::string filename, void** data, size_t* size);
        static void OnWorldPreLoadEvent(std::string mapName, bool unk1, bool modded);
};

#endif//INC_GDCL_DLL_SERVER_COORDINATOR_H