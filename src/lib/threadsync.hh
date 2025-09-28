#pragma once

#include <corba/coroutine.hh>
#include <vector>
#include <mutex>

/**
 * Helper to suspend co-routines in one thread and resume them in another thread.
 */
class ThreadSync {
    std::vector<std::coroutine_handle<>> _queue;
    std::mutex _mutex;
    std::function<void()> _schedule;
  
    struct awaitable {
        ThreadSync *_owner;
        awaitable(ThreadSync *owner) : _owner(owner) {}
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) { _owner->add(h); }
        void await_resume() { }
    };
    void add(std::coroutine_handle<> handle) {
        _mutex.lock();
        _queue.push_back(handle);
        if (_queue.size() == 1) {
            _schedule();
        }
        _mutex.unlock();
    }
  
  public:
    /**
     * schedule: function which is called during suspend() to inform about the need to resume.
     */
    ThreadSync(std::function<void()> schedule): _schedule(schedule) {}
    /**
     * call to suspend co-routine
     */
    awaitable suspend() { return awaitable{this}; }
    /**
     * resume all suspended co-routines
     */
    void resume() {
        std::vector<std::coroutine_handle<>> queue;
        _mutex.lock();
        queue.swap(_queue);
        _mutex.unlock();
        for (auto &handle : queue) {
            handle.resume();
        }
    }
};
