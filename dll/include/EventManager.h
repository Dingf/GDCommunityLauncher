#ifndef INC_GDCL_DLL_EVENT_MANAGER_H
#define INC_GDCL_DLL_EVENT_MANAGER_H

#include <map>
#include <unordered_set>

enum GDCLEvent
{
    GDCL_EVENT_CONNECT,
    GDCL_EVENT_DISCONNECT,
    GDCL_EVENT_WORLD_PRE_LOAD,
    GDCL_EVENT_WORLD_POST_LOAD,
    GDCL_EVENT_WORLD_PRE_UNLOAD,
    GDCL_EVENT_WORLD_POST_UNLOAD,
    GDCL_EVENT_DIRECT_FILE_READ,
    GDCL_EVENT_DIRECT_FILE_WRITE,
    GDCL_EVENT_SET_MAIN_PLAYER,
};

class EventManager
{
    public:
        typedef void (*EventHandler)(void*);

        static void Publish(GDCLEvent event, void* data = nullptr);
        static void Subscribe(GDCLEvent event, EventHandler handler);
        static void Unsubscribe(GDCLEvent event, EventHandler handler);

    private:
        EventManager() {};
        EventManager(EventManager&) = delete;
        void operator=(const EventManager&) = delete;

        static EventManager& GetInstance();

        std::map<GDCLEvent, std::unordered_set<EventHandler>> _handlers;
};

#endif//INC_GDCL_DLL_EVENT_MANAGER_H