#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__
#include "thread_safe_queue.h"
#include <concepts>
#include <functional>
#include <future>
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

private:
    void run_consumer();

public:
    /*
    We are not supposed to call this twice in a thread or multiple threads.
    */
    void start();
    void cancel(bool wait);
    ~thread_pool_t();
};
#endif //__THREAD_POOL_H__
