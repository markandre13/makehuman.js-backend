// #include "../src/opencv/loop.hh"

#include <corba/coroutine.hh>
#include <thread>
#include <mutex>

#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;
using namespace std;

class ThreadSync {

    CORBA::signal signal;

    struct Node {
        CORBA::signal signal;
        Node *next = nullptr;
    };
    
    // std::queue<CORBA::signal> _queue;
    CORBA::async<> _suspend();
    Node *_head = nullptr;
    Node *_tail = nullptr;
    std::mutex _mutex;
public:
    CORBA::async<> suspend();
    /**
     * suspend the caller
     * 
     * @param scheduleResumeEvent 
     */
    CORBA::async<> suspend(std::function<void()> scheduleResumeEvent);
    /**
     * resume all callers
     */
    void resume();
};

CORBA::async<> ThreadSync::_suspend() {
    _mutex.lock();
    Node *node = new Node();
    if (_tail) {
        _tail->next = node;
    }
    _tail = node;
    if (!_head) {
        _head = node;
    }
    _mutex.unlock();
    co_await node->signal.suspend();
    co_return;
}

CORBA::async<> ThreadSync::suspend() {
    // const std::lock_guard<std::mutex> lock(_mutex);
    // _queue.emplace().suspend();
    co_await _suspend();
    co_return;
}
/**
 * suspend the caller
 * 
 * @param scheduleResumeEvent 
 */
CORBA::async<> ThreadSync::suspend(std::function<void()> scheduleResumeEvent) {
    // const std::lock_guard<std::mutex> lock(_mutex);
    scheduleResumeEvent();
    // _queue.emplace().suspend();
    co_await _suspend();
    co_return;
}
/**
 * resume all callers
 */
void ThreadSync::resume() {
    _mutex.lock();
    while(_head) {
        Node *head = _head;
        _head = _head->next;
        _mutex.unlock();
        head->signal.resume();
        delete head;
        _mutex.lock();
    }
    _tail = nullptr;
    // for (; !_queue.empty(); _queue.pop()) {
    //     _queue.front().resume();
    //     // auto &front = _queue.front();
    //     // _mutex.unlock();
    //     // front.resume();
    //     // _mutex.lock();
    // }
    _mutex.unlock();
}

CORBA::async<> foo(cppasync::signal &a, cppasync::signal &b) {
    println("{}:{}: a.suspend()", __FILE__, __LINE__);
    co_await a.suspend();
    println("{}:{}: b.suspend()", __FILE__, __LINE__);
    co_await b.suspend(); // TODO: SEGV when co_await is missing. can we get a compiler warning?
    println("{}:{}: co_return", __FILE__, __LINE__);
    co_return;
}

CORBA::async<> foo2(ThreadSync &a, ThreadSync &b) {
    println("{}:{}: a.suspend()", __FILE__, __LINE__);
    co_await a.suspend();
    println("{}:{}: b.suspend()", __FILE__, __LINE__);
    co_await b.suspend(); // TODO: SEGV when co_await is missing. can we get a compiler warning?
    println("{}:{}: co_return", __FILE__, __LINE__);
    co_return;
}

static CORBA::async<> bar() {
    // println("{}:{}", __FILE__, __LINE__);
    // co_await a.suspend();
    // println("{}:{}", __FILE__, __LINE__);
    // // co_await b.suspend();
    // println("{}:{}", __FILE__, __LINE__);
    co_return;
}

kaffeeklatsch_spec([] {
    describe("class OpenCVLoop", [] {
        it("with cppasync::signal", [] {
            println("START");
            cppasync::signal a;
            cppasync::signal b;
            println("{}:{}: call foo()", __FILE__, __LINE__);
            foo(a, b).thenOrCatch(
                []() {
                    println("{}:{}: then", __FILE__, __LINE__);
                },
                [&](std::exception_ptr eptr) {
                    try {
                        std::rethrow_exception(eptr);
                    }
                    catch(runtime_error &error) {
                        println("{}:{}: catch: {}", __FILE__, __LINE__, error.what());
                    }
                });
            println("{}:{}: a.resume()", __FILE__, __LINE__);
            a.resume();
            println("{}:{}: b.resume()", __FILE__, __LINE__);
            b.resume();
            println("{}:{}: done", __FILE__, __LINE__);
        });
        it("with ThreadSync", [] {
            println("START");
            // cppasync::signal a;
            // cppasync::signal b;
            ThreadSync a;
            ThreadSync b;
            println("{}:{}: call foo()", __FILE__, __LINE__);
            foo2(a, b).thenOrCatch(
                []() {
                    println("{}:{}: then", __FILE__, __LINE__);
                },
                [&](std::exception_ptr eptr) {
                    try {
                        std::rethrow_exception(eptr);
                    }
                    catch(runtime_error &error) {
                        println("{}:{}: catch: {}", __FILE__, __LINE__, error.what());
                    }
                });
            println("{}:{}: a.resume()", __FILE__, __LINE__);
            a.resume();
            println("{}:{}: b.resume()", __FILE__, __LINE__);
            b.resume();
            println("{}:{}: done", __FILE__, __LINE__);
        });
        xit("execute", [] {
            // println("{}:{}", __FILE__, __LINE__);
            // bar().then([] {
            //     println("{}:{}", __FILE__, __LINE__);
            //     println("DONE");
            // });
            // println("{}:{}", __FILE__, __LINE__);
            // a.resume();
            // println("{}:{}", __FILE__, __LINE__);
            // // b.resume();
            // println("{}:{}", __FILE__, __LINE__);

            // std::println("OpenCVLoop::execute(): SUSPEND OPENCV");
            // // wait for this function to be executed from within the OpenCV thread
            // _syncOpenCV.suspend([&]{resume();});
            // std::println("OpenCVLoop::execute(): RESUME OPENCV");
            // T result = cmd();
            // // wait for this function to be executed from within the libev thread
            // std::println("OpenCVLoop::execute(): SUSPEND LIBEV");
            // _waitForLibEVLoop.suspend([&]{ev_async_send(_loop, &asyncWatcher.watcher);});
            // std::println("OpenCVLoop::execute(): RESUME LIBEV");
            // co_return result;
        });
    });
});
