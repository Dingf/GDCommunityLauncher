#ifndef INC_GDCL_THREAD_SAFE_QUEUE_H
#define INC_GDCL_THREAD_SAFE_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue
{
    public:
        bool empty() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _queue.empty();
        }

        size_t size() const
        {
            return _queue.size();
        }

        template <class... Ts>
        void push(Ts&&... value)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(value...);
            _condition.notify_one();
        }

        template <class... Ts>
        void emplace(Ts&&... value)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.emplace(value...);
            _condition.notify_one();
        }

        bool try_pop(T& value)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_queue.empty())
                return false;

            value = _queue.front();
            _queue.pop();
            return true;
        }

        bool front(T& value)
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_queue.empty())
                return false;

            value = _queue.front();
            return true;
        }

        void pop(T& value)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _condition.wait(lock, [this] { return !_queue.empty(); });
            value = _queue.front();
            _queue.pop();
        }

        void pop()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _condition.wait(lock, [this] { return !_queue.empty(); });
            _queue.pop();
        }

        void clear()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            std::queue<T> empty;
            _queue.swap(empty);
        }

    private:
        std::queue<T>           _queue;
        mutable std::mutex      _mutex;
        std::condition_variable _condition;
};


#endif//INC_GDCL_THREAD_SAFE_QUEUE_H