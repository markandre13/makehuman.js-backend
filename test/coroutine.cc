import std;
#include "../src/corba/coroutine.hh"

#include <bandit/bandit.h>

using namespace snowhouse;
using namespace bandit;

using namespace std;
using namespace corba;

vector<string> logger;
template <class... Args>
void log(std::format_string<Args...> fmt, Args&&... args) {
    logger.push_back(format(fmt, args...));
}

interlock<unsigned, unsigned> my_interlock;

task<const char*> f3() {
    log("f3 enter");
    auto v = co_await my_interlock.suspend(10);
    log("corba -> {}", v);
    log("f3 leave");
    co_return "hello";
}

task<> f2() {
    log("f2 enter");
    auto text = co_await f3();
    log("expect 'hello', got '{}'", text);
    log("f2 leave");
    co_return;
}

task<double> f1() {
    log("f1 enter");
    co_await f2();
    log("f1 middle");
    co_await f2();
    log("f1 leave");
    co_return 3.1415;
}

task<int> f0() {
    log("f0 enter");
    double pi = co_await f1();
    log("expect 3.1415, got {}", pi);
    log("f0 leave");
    co_return 10;
}

bool fx_done = false;
task<int> fx() {
    fx_done = true;
    co_return 10;
}

go_bandit([]() {
    describe("coroutine", []() {
        it("handles many nested co_awaits", []() {
            f0().no_wait();
            my_interlock.resume(10, 2001);
            my_interlock.resume(10, 2010);

            auto expect = vector<string> {{
                "f0 enter",
                "f1 enter",
                "f2 enter",
                "f3 enter",
                "corba -> 2001",
                "f3 leave",
                "expect 'hello', got 'hello'",
                "f2 leave",
                "f1 middle",
                "f2 enter",
                "f3 enter",
                "corba -> 2010",
                "f3 leave",
                "expect 'hello', got 'hello'",
                "f2 leave",
                "f1 leave",
                "expect 3.1415, got 3.1415",
                "f0 leave"
            }};
            AssertThat(logger, EqualsContainer(expect));
        });
        it("handles no co_await", []() {
            fx().no_wait();
            AssertThat(fx_done, Equals(true));
        });
    });
});


