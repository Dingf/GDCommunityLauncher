#include "EventManager.h"

EventManager& EventManager::GetInstance()
{
    static EventManager instance;
    return instance;
}

void EventManager::Publish(GDCLEvent event, void* data)
{
    EventManager& manager = EventManager::GetInstance();
    for (EventHandler handler: manager._handlers[event])
    {
        handler(data);
    }
}

void EventManager::Subscribe(GDCLEvent event, EventHandler handler)
{
    EventManager& manager = EventManager::GetInstance();
    manager._handlers[event].insert(handler);
}

void EventManager::Unsubscribe(GDCLEvent event, EventHandler handler)
{
    EventManager& manager = EventManager::GetInstance();
    manager._handlers[event].erase(handler);
}