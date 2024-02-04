import std;

#include <gtest/gtest.h>
#include "../src/corba/coroutine.hh"

using namespace std;
using namespace corba;

interlock<unsigned, unsigned> my_interlock;

task<const char*> f3() {
    println("f3 enter");
    auto v = co_await my_interlock.suspend(10);
    println("corba -> {}", v);
    println("f3 leave");
    co_return "hello";
}

task<> f2() {
    println("f2 enter");
    auto text = co_await f3();
    println("expect 'hello', got '{}'", text);
    println("f2 leave");
    co_return;
}

task<double> f1() {
    println("f1 enter");
    co_await f2();
    println("f1 middle");
    co_await f2();
    println("f1 leave");
    co_return 3.1415;
}

task<int> f0() {
    println("f0 enter");
    double pi = co_await f1();
    println("expect 3.1415, got {}", pi);
    println("f0 leave");
    co_return 10;
}

task<int> fx() {
    println("fx enter'n leave");
    co_return 10;
}

// TEST(Coroutine, Nested) {
int main() {
    {
        fx().no_wait();
        f0().no_wait(); 
        println("--------------------------");
        my_interlock.resume(10, 2001);
        println("--------------------------");
        my_interlock.resume(10, 2010);
        println("--------------------------");
    }
    println("..........................");
    // println("object_counter = {}", object_counter);
    return 0;
}
