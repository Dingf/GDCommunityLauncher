#ifndef INC_GDCL_DLL_UPDATE_THREAD_H
#define INC_GDCL_DLL_UPDATE_THREAD_H

#include <string>
#include <memory>
#include <thread>
#include <functional>

template <typename... Ts>
class UpdateThread
{
    public:
        typedef void(*UpdateCallback)(Ts...);

        UpdateThread(UpdateCallback func, uint64_t updateIntervalMS = 1000UL, uint64_t repeatIntervalMS = 0) : _updateTime(0), _updateIntervalMS(updateIntervalMS), _repeatIntervalMS(repeatIntervalMS), _callbackProto(func)
        {
            _thread = std::make_unique<std::thread>(&UpdateThread::Tick, this);
            _thread->detach();
        }

        ~UpdateThread() { Stop(); }

        bool IsRunning() const { return (_callbackProto != nullptr); }

        void Stop() { _callbackProto = nullptr; }

        void Update(uint64_t delay, Ts... args)
        {
            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            _updateTime = ms.count() + delay;

            auto f = std::bind(std::forward<UpdateCallback>(_callbackProto), std::forward<Ts>(args)...);
            _callback = [f]{ f(); };
        }

    private:
        void Tick()
        {
            while (IsRunning())
            {
                std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
                if ((_updateTime > 0) && ((uint64_t)ms.count() >= _updateTime))
                {
                    if (_callback)
                    {
                        _callback();
                    }

                    if (_repeatIntervalMS > 0)
                    {
                        _updateTime += _repeatIntervalMS;
                    }
                    else
                    {
                        _callback = nullptr;
                        _updateTime = 0;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(_updateIntervalMS));
            }
        }

        uint64_t _updateTime;
        uint64_t _updateIntervalMS;
        uint64_t _repeatIntervalMS;

        UpdateCallback _callbackProto;
        std::function<void()> _callback;

        std::unique_ptr<std::thread> _thread;
};

#endif//INC_GDCL_DLL_UPDATE_THREAD_H