#ifndef INC_GDCL_DLL_THREAD_MANAGER_H
#define INC_GDCL_DLL_THREAD_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <future>
#include <atomic>

class ThreadManager
{
    public:
        ~ThreadManager();

        template <typename F, typename... Ts>
        static bool CreateThread(const std::string& name, uint64_t tickRate, uint64_t delay, F&& callback, Ts&&... args)
        {
            ThreadManager& manager = GetInstance();

            auto it = manager._threads.find(name);
            if (it != manager._threads.end())
            {
                if (it->second->_running)
                    return false;
                else
                    manager._threads.erase(it);
            }

            TimerThread* thread = new TimerThread;
            thread->_nextTick = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() + delay;
            thread->_tickRate = tickRate;

            auto f = std::bind(std::forward<F>(callback), std::forward<Ts>(args)...);
            thread->_callback = [f] { return f(); };

            std::unique_ptr<TimerThread> threadPointer(thread);
            manager._threads.emplace(name, std::move(threadPointer));
            thread->Start();
            return true;
        }

        static void DeleteThread(const std::string& name);
        static void DeleteAllThreads();

    private:
        struct TimerThread
        {
            ~TimerThread() { Stop(); }

            void Start();
            void Stop();
            void Tick();

            std::function<int64_t()> _callback;
            std::future<void> _future;
            uint64_t _nextTick;
            uint64_t _tickRate;
            std::atomic_bool _running;
        };

        ThreadManager() {};
        ThreadManager(ThreadManager&) = delete;
        void operator=(const ThreadManager&) = delete;

        static ThreadManager& GetInstance();

        std::unordered_map<std::string, std::unique_ptr<TimerThread>> _threads;
};

#endif//INC_GDCL_DLL_THREAD_MANAGER_H