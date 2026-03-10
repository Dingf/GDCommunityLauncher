#include "ThreadManager.h"

ThreadManager::~ThreadManager()
{
    DeleteAllThreads();
}

ThreadManager& ThreadManager::GetInstance()
{
    static ThreadManager instance;
    return instance;
}

void ThreadManager::TimerThread::Start()
{
    _running = true;
    _future = std::async(std::launch::async, &TimerThread::Tick, this);
}

void ThreadManager::TimerThread::Stop()
{
    _running = false;
}

void ThreadManager::TimerThread::Tick()
{
    do
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        if ((uint64_t)ms.count() >= _nextTick)
        {
            int64_t delay = _callback();
            if (delay <= 0)
            {
                _running = false;
                break;
            }
            _nextTick += delay;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(_tickRate));
    }
    while (_running);
}

void ThreadManager::DeleteThread(const std::string& name)
{
    ThreadManager& manager = GetInstance();
    auto it = manager._threads.find(name);
    if (it != manager._threads.end())
    {
        it->second->Stop();
        manager._threads.erase(it);
    }
}

void ThreadManager::DeleteAllThreads()
{
    ThreadManager& manager = GetInstance();
    for (auto& pair : manager._threads)
    {
        pair.second->Stop();
    }
    manager._threads.clear();
}