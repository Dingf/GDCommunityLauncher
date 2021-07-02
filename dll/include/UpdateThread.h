#ifndef INC_GDCL_DLL_UPDATE_THREAD_H
#define INC_GDCL_DLL_UPDATE_THREAD_H

#include <memory>
#include <thread>

typedef void(*UpdateCallback)(void);

class UpdateThread
{
    public:
        UpdateThread(UpdateCallback callback, uint64_t intervalMS);
        ~UpdateThread() { Stop(); }

        bool IsRunning() const { return (_callback == nullptr); }

        void Stop() { _callback = nullptr; }

        void Update(uint64_t delay);

    private:
        void Tick();

        uint64_t _updateTime;
        uint64_t _updateIntervalMS;

        UpdateCallback _callback;

        std::unique_ptr<std::thread> _thread;
};

#endif//INC_GDCL_DLL_UPDATE_THREAD_H