#include <chrono>
#include "UpdateThread.h"

UpdateThread::UpdateThread(UpdateCallback callback, uint64_t intervalMS) : _updateTime(0), _updateIntervalMS(intervalMS), _callback(callback)
{
    _thread = std::make_unique<std::thread>(&UpdateThread::Tick, this);
    _thread->detach();
}

void UpdateThread::Update(uint64_t delay)
{
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
    _updateTime = ms.count() + delay;
}

void UpdateThread::Tick()
{
    while (_callback)
    {
        std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        if ((_updateTime > 0) && ((uint64_t)ms.count() >= _updateTime))
        {
            _callback();
            _updateTime = 0;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(_updateIntervalMS));
    }
}