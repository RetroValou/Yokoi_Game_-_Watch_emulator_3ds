#include <cmath>
#include <algorithm>

double random(int t)
{
    return std::fmod(std::fmod(std::sin(t), 1.0) + 1.0, 1.0);
}

int randrange(int t, int a, int b)
{
    if (a > b)
        std::swap(a, b);

    int value = static_cast<int>(random(t) * (b - a + 1) + a);
    return std::max(std::min(value, b), a);
}