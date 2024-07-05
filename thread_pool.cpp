#include <concepts>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
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
class spin_lock_t
{
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;

public:
    void lock()
    {
        while (flag.test_and_set(std::memory_order_acquire))
        {
        }
    }

    bool try_lock() { return !flag.test_and_set(std::memory_order_acquire); }
    void unlock() { flag.clear(std::memory_order_release); }
};
template <class T> struct thread_safe_queue_t
{
    spin_lock_t tex;
    std::queue<T> q;
    void push(T &&v)
    {
        std::lock_guard lg(tex);
        q.push(v);
    }
    auto pop() -> std::optional<T>
    {
        std::lock_guard lg(tex);
        if (q.empty())
        {
            return {};
        }
        std::optional<T> res = std::move(q.front());
        q.pop();
        return res;
    }
};
struct thread_pool_t
{
private:
    thread_safe_queue_t<std::function<void()>> queue;
    std::size_t num_of_workers;
    std::vector<std::thread> pool;
    bool started;
    std::atomic_flag cancelled;

public:
    thread_pool_t(std::size_t num_of_workers) : num_of_workers(num_of_workers)
    {
    }
    template <class Func, class... Args>
    auto submit(Func f, Args &&...args)
        requires std::invocable<Func, Args&&...>
    {
        // Remove shared_ptr after we have std::move_only_function from c++23.
        auto shared_task = std::make_shared<decltype(std::packaged_task(f))>(f);
        auto result = shared_task->get_future();
        // All args are copied.
        queue.push([task = std::move(shared_task), args = std::make_tuple(std::forward<Args>(args)...)]() mutable
                   { std::apply(*task, std::move(args)); });
        return result;
    }
    void run_consumer()
    {
        while (!cancelled.test(std::memory_order_acquire))
        {
            auto task = queue.pop();
            if (task.has_value())
            {
                task.value()();
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }
    /*
    We are not supposed to call this twice in a thread or multiple threads.
    */
    void start()
    {
        if (started)
        {
            return;
        }
        pool.reserve(num_of_workers);
        for (std::size_t i = 0; i < num_of_workers; ++i)
        {
            pool.emplace_back([this]() { run_consumer(); });
        }
        started = true;
    }
    void cancel(bool wait)
    {
        cancelled.test_and_set(std::memory_order_release);
        if (wait)
        {
            for (auto &thread : pool)
            {
                thread.join();
            }
        }
    }
    ~thread_pool_t() { cancel(true); }
};

int f(int a, int b) { return a + b; }

std::string f2(int a, int b, int c)
{
    return std::to_string(a) + std::to_string(b) + std::to_string(c);
}

int main()
{
    thread_pool_t thread_pool(1);
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
    //     { return a.val + b; }, move_only_class<std::string>(std::string("kk")), "bb");
    thread_pool.start();
    std::cout << "res1 = " << res1.get() << "\n";
    std::cout << "res2 = " << res2.get() << "\n";
    std::cout << "res3 = " << res3.get() << "\n";
    std::cout << "res4 = " << sum << "\n";
    // std::cout << "res6 = " << res6.get() << "\n";
}