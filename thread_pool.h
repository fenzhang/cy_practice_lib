#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
#include "thread_safe_queue.h"
#include <concepts>
#include <functional>
#include <future>
#include <optional>
#include <utility>
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
        requires std::invocable<Func, Args &&...>
    {
        // Remove shared_ptr after we have std::move_only_function from c++23.
        auto shared_task = std::make_shared<decltype(std::packaged_task(f))>(f);
        auto result = shared_task->get_future();
        // All args are copied.
        queue.push(
            [task = std::move(shared_task),
             args = std::make_tuple(std::forward<Args>(args)...)]() mutable
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
#endif //__THREAD_POOL_H__
