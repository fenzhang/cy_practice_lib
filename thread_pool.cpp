#include "thread_pool.h"
void thread_pool_t::run_consumer()
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
void thread_pool_t::start()
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
void thread_pool_t::cancel(bool wait)
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
thread_pool_t::~thread_pool_t() { cancel(true); }
