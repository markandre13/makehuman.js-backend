#pragma once

#include <coroutine>
#include <map>
#include <exception>
#include <stdexcept>

//
// "You can have peace. Or you can have freedom. Don't ever count on having both at once."
//                                                                  -- Robert A. Heinlein.
//
// while C++ 20/23 coroutines provide a lot of freedom to accommodate many different use
// cases and hardware platforms, actually _using_ them does not provide much peace of mind.
//
// the C++ coroutine support below is a bare minimum for javascript style await's and provides
// the two classes 'task' and 'interlock'.
//
// TASK
//
// 'task' is the class to be returned from coroutines.
//
// it is basically a copy of https://github.com/andreasbuhr/cppcoro's task with two changes:
//
// * while one can call a coroutine from other coroutines like this
//     task<> fun0() {
//       co_await fun1();
//     }
//   one can also call a coroutine from synchronous code
//     int fun0() {
//       fun1().no_wait();
//     }
//   with no_wait() preventing the coroutine's state from being deleted.
//   instead it will be deleted once the coroutine is finished.
// * the coroutine is immediately executed
//
// INTERLOCK
//
// 'interlock<KEY, VALUE>' is the class to be used to suspend and resume coroutines:
//
//   auto value = co_await interlock.suspend(key);
//
// will suspend the execution of the coroutine until
//
//   interlock.resume(key, value);
//
// resumes it along with providing a value.
//

namespace corba {

template <typename T>
class task;

class broken_promise : public std::logic_error {
    public:
        broken_promise() : std::logic_error("broken promise") {}
        explicit broken_promise(const std::string& what) : logic_error(what) {}
        explicit broken_promise(const char* what) : logic_error(what) {}
};

class broken_resume : public std::logic_error {
    public:
        broken_resume() : std::logic_error("broken resume") {}
        explicit broken_resume(const std::string& what) : logic_error(what) {}
        explicit broken_resume(const char* what) : logic_error(what) {}
};

namespace detail {

class task_promise_base {
        std::coroutine_handle<> m_continuation;
        struct final_awaitable {
                bool await_ready() const noexcept { return false; }
                template <typename PROMISE>
                std::coroutine_handle<> await_suspend(std::coroutine_handle<PROMISE> coro) noexcept {
                    auto continuation = coro.promise().m_continuation;
                    if (!continuation) {
                        coro.destroy();
                        return std::noop_coroutine();
                    }
                    coro.promise().m_continuation = nullptr;
                    return continuation;
                }
                void await_resume() noexcept {}
        };

    public:
        task_promise_base() noexcept {}
        // set the coroutine to proceed with after this coroutine is finished
        void set_continuation(std::coroutine_handle<> continuation) noexcept { m_continuation = continuation; }

        std::suspend_never initial_suspend() { return {}; }
        final_awaitable final_suspend() noexcept { return {}; }
};

// the VALUE specialisation of task_promise_base
template <typename T>
class task_promise final : public task_promise_base {
    public:
        task_promise() noexcept {}
        ~task_promise() {
            switch (m_resultType) {
                case result_type::value:
                    m_value.~T();
                    break;
                case result_type::exception:
                    m_exception.~exception_ptr();
                    break;
                default:
                    break;
            }
        }
        task<T> get_return_object() noexcept;
        void unhandled_exception() noexcept {
            ::new (static_cast<void*>(std::addressof(m_exception))) std::exception_ptr(std::current_exception());
            m_resultType = result_type::exception;
        }

        template <typename VALUE, typename = std::enable_if_t<std::is_convertible_v<VALUE&&, T>>>
        void return_value(VALUE&& value) noexcept(std::is_nothrow_constructible_v<T, VALUE&&>) {
            ::new (static_cast<void*>(std::addressof(m_value))) T(std::forward<VALUE>(value));
            m_resultType = result_type::value;
        }

        T& result() & {
            if (m_resultType == result_type::exception) {
                std::rethrow_exception(m_exception);
            }
            assert(m_resultType == result_type::value);
            return m_value;
        }

        using rvalue_type = std::conditional_t<std::is_arithmetic_v<T> || std::is_pointer_v<T>, T, T&&>;
        rvalue_type result() && {
            if (m_resultType == result_type::exception) {
                std::rethrow_exception(m_exception);
            }
            assert(m_resultType == result_type::value);
            return std::move(m_value);
        }

    private:
        enum class result_type { empty, value, exception };
        result_type m_resultType = result_type::empty;
        union {
                T m_value;
                std::exception_ptr m_exception;
        };
};

// the VOID specialisation of task_promise_base
template <>
class task_promise<void> : public task_promise_base {
    public:
        task_promise() noexcept = default;
        task<void> get_return_object() noexcept;
        void return_void() noexcept {}
        void unhandled_exception() noexcept { m_exception = std::current_exception(); }
        void result() {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
        }

    private:
        std::exception_ptr m_exception;
};

// the REFERENCE specialisation of task_promise_base
template <typename T>
class task_promise<T&> : public task_promise_base {
    public:
        task_promise() noexcept = default;
        task<T&> get_return_object() noexcept;
        void unhandled_exception() noexcept { m_exception = std::current_exception(); }
        void return_value(T& value) noexcept { m_value = std::addressof(value); }
        T& result() {
            if (m_exception) {
                std::rethrow_exception(m_exception);
            }
            return *m_value;
        }

    private:
        T* m_value = nullptr;
        std::exception_ptr m_exception;
};
}  // namespace detail

template <typename T = void>
class [[nodiscard]] task {
    public:
        using promise_type = detail::task_promise<T>;
        using handle_type = std::coroutine_handle<promise_type>;
        using value_type = T;

    private:
        handle_type m_coroutine;
        struct awaitable_base {
                handle_type m_coroutine;
                awaitable_base(handle_type coroutine) noexcept : m_coroutine(coroutine) {}
                bool await_ready() const noexcept { return false; }
                void await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept { m_coroutine.promise().set_continuation(awaitingCoroutine); }
        };

    public:
        task() noexcept : m_coroutine(nullptr) {}
        explicit task(handle_type coroutine) : m_coroutine(coroutine) {}
        task(task&& t) noexcept : m_coroutine(t.m_coroutine) { t.m_coroutine = nullptr; }

    private:
        task(const task&) = delete;
        task& operator=(const task&) = delete;

    public:
        ~task() {
            if (m_coroutine) {
                m_coroutine.destroy();
            }
        }

        task& operator=(task&& other) noexcept {
            if (std::addressof(other) != this) {
                if (m_coroutine) {
                    m_coroutine.destroy();
                }
                m_coroutine = other.m_coroutine;
                other.m_coroutine = nullptr;
            }
            return *this;
        }

        auto operator co_await() const& noexcept {
            struct awaitable : awaitable_base {
                    using awaitable_base::awaitable_base;
                    decltype(auto) await_resume() {
                        if (!this->m_coroutine) {
                            throw broken_promise{};
                        }
                        return this->m_coroutine.promise().result();
                    }
            };
            return awaitable{m_coroutine};
        }
        auto operator co_await() const&& noexcept {
            struct awaitable : awaitable_base {
                    using awaitable_base::awaitable_base;
                    decltype(auto) await_resume() {
                        if (!this->m_coroutine) {
                            throw broken_promise{};
                        }
                        return std::move(this->m_coroutine.promise()).result();
                    }
            };
            return awaitable{m_coroutine};
        }
        bool is_ready() const noexcept { return !m_coroutine || m_coroutine.done(); }
        void no_wait() { m_coroutine = nullptr; }
};

namespace detail {

template <typename T>
task<T> task_promise<T>::get_return_object() noexcept {
    return task<T>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

inline task<void> task_promise<void>::get_return_object() noexcept { return task<void>{std::coroutine_handle<task_promise>::from_promise(*this)}; }

template <typename T>
task<T&> task_promise<T&>::get_return_object() noexcept {
    return task<T&>{std::coroutine_handle<task_promise>::from_promise(*this)};
}

}  // namespace detail

template <typename K, typename V>
class interlock {
    private:
        class awaiter {
            public:
                awaiter(K id, interlock* _this) : id(id), _this(_this) {}
                bool await_ready() const noexcept { return false; }
                template <typename T>
                bool await_suspend(std::coroutine_handle<detail::task_promise<T>> awaitingCoroutine) noexcept {
                    _this->suspended[id] = *((std::coroutine_handle<detail::task_promise_base>*)&awaitingCoroutine);
                    return true;
                }
                V await_resume() {
                    auto it = _this->result.find(id);
                    if (it == _this->result.end()) {
                        throw broken_resume("broken resume: did not find value");
                    }
                    return it->second;
                }

            private:
                K id;
                interlock* _this;
        };

        // TODO: use only one map
        std::map<K, std::coroutine_handle<detail::task_promise_base>> suspended;
        std::map<K, V> result;

    public:
        auto suspend(K id) { return awaiter{id, this}; }
        void resume(K id, V result) {
            auto it = suspended.find(id);
            if (it == suspended.end()) {
                throw broken_resume("broken resume: did not find task");
            }
            auto continuation = it->second;
            suspended.erase(it);
            if (!continuation.done()) {
                this->result[id] = result;
                continuation.resume();
            }
        }
};

}  // namespace corba