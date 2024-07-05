#include "thread_pool.h"
#include <iostream>
#include <string>
#include <utility>
template <class T> struct move_only_class
{
    T val;
    move_only_class(T v) : val(v) {}
    move_only_class(move_only_class &&o) : val(std::move(o.val)) {}
    auto operator=(move_only_class &&o) -> decltype(*this)
    {
        val = std::move(o.val);
        return *this;
    }
    move_only_class(const move_only_class &o) = delete;
    auto operator=(const move_only_class &o) -> decltype(*this) = delete;
};
template <class T> struct copy_only_class
{
    T val;
    copy_only_class(T v) : val(v) {}
    copy_only_class(copy_only_class &&o) = delete;
    auto operator=(copy_only_class &&o) = delete;
    copy_only_class(const copy_only_class &o) : val(o.val) {}
    auto operator=(const copy_only_class &o) -> decltype(*this)
    {
        val = o.val;
        return *this;
    }
};

int f(int a, int b) { return a + b; }

std::string f2(int a, int b, int c)
{
    return std::to_string(a) + std::to_string(b) + std::to_string(c);
}

int main()
{
    thread_pool_t thread_pool(4);
    auto res1 = thread_pool.submit(f, 1, 2);
    auto res2 = thread_pool.submit(f2, 3, 4, 5);
    auto res3 = thread_pool.submit([](int a, int b) { return a * b; }, 6, 7);
    int sum = 0;
    auto res4 = thread_pool.submit(
        [&sum](int a, int b) mutable
        {
            sum += a;
            sum += b;
        },
        6, 7);
    // auto res6 = thread_pool.submit(
    //     [](move_only_class<std::string> a, const std::string &b) mutable
    //     { return a.val + b; },
    //     move_only_class<std::string>(std::string("kk")), "bb");
    thread_pool.start();
    std::cout << "res1 = " << res1.get() << "\n";
    std::cout << "res2 = " << res2.get() << "\n";
    std::cout << "res3 = " << res3.get() << "\n";
    std::cout << "res4 = " << sum << "\n";
    // std::cout << "res6 = " << res6.get() << "\n";
}
