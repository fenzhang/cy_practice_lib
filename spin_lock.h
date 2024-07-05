#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__
#include <future>
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
#endif //__SPIN_LOCK_H__
