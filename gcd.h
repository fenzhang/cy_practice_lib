#ifndef __GCD_H__
#define __GCD_H__
#include <exception>
#include <concepts>
namespace cy
{
    /*
    Return the maximum value v such that a % v == T{} and b % v == T{}.

    If both a == T{} and b == T{} then return T{}.
    */
    template <std::integral T> T gcd(T a, T b)
    {
        // 5 3
        // 3 2
        // 2 1
        // 1 0
        while (b != T{})
        {
            auto new_a = b;
            auto new_b = a % b;
            std::swap(a, new_a);
            std::swap(b, new_b);
        }
        return a;
    }
} // namespace cy
#endif // __GCD_H__