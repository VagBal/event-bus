#ifndef RAND_H
#define RAND_H

#include <cstdint>
#include <cassert>

namespace Util
{

/*
    @class Rand
    @brief The Rand class provides a simple pseudorandom number generator 
    based on the XorShift32 algorithm. It offers methods to generate uniformly 
    distributed integers, check for one-in-n probability, and generate skewed values.
    @author vabarob
    @date 2026-01
*/
class RandomNumberGenerator
{
public:
    explicit RandomNumberGenerator(uint32_t seed) : seed_(seed)
    {
        if (seed_ == 0)
        {
            seed_ = 1; // Fallback to a default non-zero seed
        }
    }

    uint32_t uniform_dist(int n)
    {
        if (n <= 0)
        {
            assert(false && "n must be positive");
            return 0;
        }

        if ((n & (n - 1)) == 0)
        {
            return xorshift32() & (n - 1);
        }

        return (static_cast<uint64_t>(xorshift32()) * static_cast<uint64_t>(n)) >> 32;
    }

    bool one_in(int n)
    {
        if (n <= 0)
        {
            assert(false && "n must be positive");
            return false;
        }
        return (uniform_dist(n) == 0);
    }

    uint32_t skewed(int max_log)
    {
        if (max_log < 0)
        {
            assert(false && "max_log must be non-negative");
            return 0;
        }
        return uniform_dist(1 << uniform_dist(max_log + 1));
    }

private:
    uint32_t xorshift32()
    {
        // XorShift32 algorithm
        seed_ ^= (seed_ << 13);
        seed_ ^= (seed_ >> 17);
        seed_ ^= (seed_ << 5);
        return seed_;
    }

    uint32_t seed_;
};

} // namespace Util

#endif // RAND_H