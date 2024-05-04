#include "EventManager.h"

EventManager& EventManager::GetInstance()
{
    static EventManager instance;
    return instance;
}

void EventManager::Subscribe(GDCLEvent event, void* handler)
{
    EventManager& manager = EventManager::GetInstance();
    manager._handlers[event].insert(handler);
}

void EventManager::Unsubscribe(GDCLEvent event, void* handler)
{
    EventManager& manager = EventManager::GetInstance();
    manager._handlers[event].erase(handler);
}