#include "EventManager.h"

EventManager& EventManager::GetInstance()
{
    static EventManager instance;
    return instance;
}

void EventManager::Subscribe(GDCLEvent event, void* handler)
{
    GetInstance()._handlers[event].insert(handler);
}

void EventManager::Unsubscribe(GDCLEvent event, void* handler)
{
    GetInstance()._handlers[event].erase(handler);
}