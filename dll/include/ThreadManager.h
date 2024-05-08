#ifndef INC_GDCL_DLL_THREAD_MANAGER_H
#define INC_GDCL_DLL_THREAD_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <thread>
#include <functional>

class ThreadManager
{
    public:
        ~ThreadManager();

        template <typename F, typename... Ts>
        static bool CreateThread(const std::string& name, F&& callback, Ts&&... args)
        {
            ThreadManager& manager = GetInstance();
            BaseThread* thread;
            if (manager._threads.count(name) == 0)
            {
                thread = new BaseThread;
            }
            else
            {
                ThreadType type = manager._threads[name]->GetType();
                if (type == THREAD_TYPE_BASE)
                    thread = manager._threads[name].get();
                else
                    return false;
            }

            thread->_name = name;

            auto f = std::bind(std::forward<F>(callback), std::forward<Ts>(args)...);
            thread->_callback = [f] { f(); };

            if (manager._threads.count(name) == 0)
            {
                std::unique_ptr<BaseThread> threadPointer(thread);
                manager._threads.emplace(name, std::move(threadPointer));
            }
            else
            {
                thread->Start();
            }
            return true;
        }

        template <typename F, typename... Ts>
        static bool CreatePeriodicThread(const std::string& name, uint64_t tickRate, uint64_t repeat, uint64_t delay, F&& callback, Ts&&... args)
        {
            ThreadManager& manager = GetInstance();
            PeriodicThread* thread;
            if (manager._threads.count(name) == 0)
            {
                thread = new PeriodicThread;
            }
            else
            {
                ThreadType type = manager._threads[name]->GetType();
                if (type == THREAD_TYPE_PERIODIC)
                    thread = dynamic_cast<PeriodicThread*>(manager._threads[name].get());
                else
                    return false;
            }

            thread->_name = name;

            std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
            thread->_updateTime = ms.count() + delay;
            thread->_tickRate = tickRate;
            thread->_repeat = repeat;

            auto f = std::bind(std::forward<F>(callback), std::forward<Ts>(args)...);
            thread->_callback = [f] { f(); };

            if (manager._threads.count(name) == 0)
            {
                std::unique_ptr<BaseThread> threadPointer(thread);
                manager._threads.emplace(name, std::move(threadPointer));
            }
            else
            {
                thread->Start();
            }
            return true;
        }

        static void StopThread(const std::string& name);

    private:
        enum ThreadType
        {
            THREAD_TYPE_BASE = 0,
            THREAD_TYPE_PERIODIC = 1,
        };

        struct BaseThread
        {
            BaseThread();
            virtual ~BaseThread() { Stop(); }

            void Callback();

            void Start();
            void Stop();
            virtual void Update();
            virtual ThreadType GetType() { return THREAD_TYPE_BASE; }

            std::string _name;
            std::function<void()> _callback;
            std::unique_ptr<std::thread> _thread;
        };

        struct PeriodicThread : public BaseThread
        {
            void Update();
            ThreadType GetType() { return THREAD_TYPE_PERIODIC; }

            uint64_t _updateTime;
            uint64_t _tickRate;
            uint64_t _repeat;
        };

        ThreadManager() {};
        ThreadManager(ThreadManager&) = delete;
        void operator=(const ThreadManager&) = delete;

        static ThreadManager& GetInstance();

        void DeleteThread(BaseThread* thread);

        std::unordered_map<std::string, std::unique_ptr<BaseThread>> _threads;
};

#endif//INC_GDCL_DLL_THREAD_MANAGER_H