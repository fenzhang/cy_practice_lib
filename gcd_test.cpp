#include "gcd.h"
#include <cassert>
#include <cmath>
int main()
{
    assert(cy::gcd(0, 1) == 1);
    assert(cy::gcd(1, 0) == 1);
    assert(cy::gcd(6, 10) == 2);
    assert(cy::gcd(10, 15) == 5);
    assert(cy::gcd(15, 6) == 3);
    assert(std::abs(cy::gcd(-6, 10)) == 2);
    // The following code results in compile error.
    // assert(cy::gcd(15.0, 6.0) == 3);
}
