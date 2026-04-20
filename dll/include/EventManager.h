#ifndef INC_GDCL_DLL_EVENT_MANAGER_H
#define INC_GDCL_DLL_EVENT_MANAGER_H

#include <map>
#include <unordered_set>

enum GDCLEvent
{
    GDCL_EVENT_INITIALIZE,
    GDCL_EVENT_PRE_SHUTDOWN,
    GDCL_EVENT_POST_SHUTDOWN,
    GDCL_EVENT_EXIT_PLAYING_MODE,
    GDCL_EVENT_WORLD_PRE_LOAD,
    GDCL_EVENT_WORLD_POST_LOAD,
    GDCL_EVENT_WORLD_PRE_UNLOAD,
    GDCL_EVENT_WORLD_POST_UNLOAD,
    GDCL_EVENT_DIRECT_FILE_READ,
    GDCL_EVENT_DIRECT_FILE_WRITE,
    GDCL_EVENT_ADD_SAVE_JOB,
    GDCL_EVENT_SET_MAIN_PLAYER,
    GDCL_EVENT_SET_SEASON_PLAYER,
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
    GDCL_EVENT_KEY_BUTTON_EVENT,
    GDCL_EVENT_BESTOW_TOKEN,
    GDCL_EVENT_CARAVAN_INTERACT,
};

class EventManager
{
    public:
        // Publishes an event to all handlers subscribed to the event
        template <typename... Ts>
        static void Publish(GDCLEvent event, Ts... args)
        {
            typedef void (__thiscall* EventHandlerProto)(Ts...);
            for (void* handler : GetInstance()._handlers[event])
            {
                ((EventHandlerProto)handler)(args...);
            }
        }

        // Publishes an event to all handlers subscribed to the event and checks the results
        // Returns true if there is at least one handler and all of the handlers returned true, otherwise returns false
        template <typename... Ts>
        static bool Poll(GDCLEvent event, Ts... args)
        {
            const auto& handlers = GetInstance()._handlers[event];
            bool result = (handlers.size() > 0);

            typedef bool (__thiscall* EventHandlerProto)(Ts...);
            for (void* handler : handlers)
            {
                if (!((EventHandlerProto)handler)(args...))
                    result = false;
            }
            return result;
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