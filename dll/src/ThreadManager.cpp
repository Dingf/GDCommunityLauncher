#include "ThreadManager.h"

ThreadManager::~ThreadManager()
{
    _threads.clear();
}

ThreadManager& ThreadManager::GetInstance()
{
    static ThreadManager instance;
    return instance;
}

void ThreadManager::StopThread(const std::string& name)
{
    ThreadManager& manager = GetInstance();
    if (manager._threads.count(name) > 0)
        manager._threads[name]->Stop();
}

void ThreadManager::DeleteThread(BaseThread* thread)
{
    ThreadManager& manager = GetInstance();
    for (auto& pair : _threads)
    {
        if (pair.second.get() == thread)
        {
            _threads.erase(pair.first);
            break;
        }
    }
}

ThreadManager::BaseThread::BaseThread()
{
    Start();
}

void ThreadManager::BaseThread::Start()
{
    _thread = std::thread(&BaseThread::Callback, this);
    _thread.detach();
}

void ThreadManager::BaseThread::Stop()
{
    _callback = nullptr;
}

void ThreadManager::BaseThread::Callback()
{
    while (_callback != nullptr)
        Update();
}

void ThreadManager::BaseThread::Update()
{
    if (_callback != nullptr)
    {
        _callback();
        Stop();
    }
}

void ThreadManager::PeriodicThread::Update()
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    if ((_updateTime > 0) && ((uint64_t)ms.count() >= _updateTime))
    {
        if (_callback != nullptr)
        {
            _callback();
            _updateTime += _repeat;

            if (_repeat == 0)
            {
                Stop();
                return;
            }
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(_tickRate));
}