#ifndef INC_GDCL_DLL_EVENT_MANAGER_H
#define INC_GDCL_DLL_EVENT_MANAGER_H

#include <map>
#include <unordered_set>

enum GDCLEvent
{
    GDCL_EVENT_INITIALIZE,
    GDCL_EVENT_SHUTDOWN,
    GDCL_EVENT_WORLD_PRE_LOAD,
    GDCL_EVENT_WORLD_POST_LOAD,
    GDCL_EVENT_WORLD_PRE_UNLOAD,
    GDCL_EVENT_WORLD_POST_UNLOAD,
    GDCL_EVENT_DIRECT_FILE_READ,
    GDCL_EVENT_DIRECT_FILE_WRITE,
    GDCL_EVENT_ADD_SAVE_JOB,
    GDCL_EVENT_SET_MAIN_PLAYER,
    GDCL_EVENT_TRANSFER_PRE_LOAD,
    GDCL_EVENT_TRANSFER_POST_LOAD,
    GDCL_EVENT_TRANSFER_PRE_SAVE,
    GDCL_EVENT_TRANSFER_POST_SAVE,
    GDCL_EVENT_CHARACTER_PRE_LOAD,
    GDCL_EVENT_CHARACTER_POST_LOAD,
    GDCL_EVENT_CHARACTER_PRE_SAVE,
    GDCL_EVENT_CHARACTER_POST_SAVE,
    GDCL_EVENT_APPLY_DAMAGE,
    GDCL_EVENT_DELETE_FILE,
};

class EventManager
{
    public:
        template <typename... Ts>
        static void Publish(GDCLEvent event, Ts... args)
        {
            typedef bool (__thiscall* EventHandlerProto)(Ts...);
            for (void* handler : GetInstance()._handlers[event])
            {
                ((EventHandlerProto)handler)(args...);
            }
        }

        static void Subscribe(GDCLEvent event, void* handler);
        static void Unsubscribe(GDCLEvent event, void* handler);

    private:
        EventManager() {};
        EventManager(EventManager&) = delete;
        void operator=(const EventManager&) = delete;

        static EventManager& GetInstance();

        std::map<GDCLEvent, std::unordered_set<void*>> _handlers;
};

#endif//INC_GDCL_DLL_EVENT_MANAGER_H