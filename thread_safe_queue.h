#ifndef __THREAD_SAFE_QUEUE_H__
#define __THREAD_SAFE_QUEUE_H__
#include "spin_lock.h"
#include <mutex>
#include <optional>
#include <queue>
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
#endif //__THREAD_SAFE_QUEUE_H__
