#ifndef RAND_H
#define RAND_H

#include <cstdint>
#include <cassert>

namespace Util
{

/**
 * @class RandomNumberGenerator
 * @brief Fast pseudorandom number generator using XorShift32 algorithm
 * 
 * This class provides a simple yet efficient pseudorandom number generator based on
 * the XorShift32 algorithm. It offers methods to generate uniformly distributed integers,
 * check for one-in-n probability events, and generate skewed distributions.
 * 
 * The generator is designed for simulation purposes and provides good statistical properties
 * while maintaining high performance through bitwise operations.
 * 
 * @author vabarob
 * @date 2026-01
 */
class RandomNumberGenerator
{
public:
    /**
     * @brief Constructs a random number generator with the specified seed
     * @param seed Initial seed value for the generator. If zero, defaults to 1.
     * 
     * The seed must be non-zero for the XorShift algorithm to function properly.
     * If a zero seed is provided, it will be automatically changed to 1.
     */
    explicit RandomNumberGenerator(uint32_t seed) : seed_(seed)
    {
        if (seed_ == 0)
        {
            seed_ = 1; // Fallback to a default non-zero seed
        }
    }

    /**
     * @brief Generates a uniformly distributed random integer in the range [0, n)
     * @param n Upper bound (exclusive) for the random number range. Must be positive.
     * @return Random integer in [0, n), or 0 if n <= 0
     * 
     * Uses optimized bitwise operations for power-of-two ranges, otherwise
     * uses multiplication and bit shifting for uniform distribution.
     * 
     * @warning Asserts if n <= 0 in debug builds
     */
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

    /**
     * @brief Returns true with 1/n probability
     * @param n Denominator of probability. Must be positive.
     * @return true with probability 1/n, false otherwise
     * 
     * This method is useful for simulating rare events. For example, one_in(100)
     * returns true approximately 1% of the time.
     * 
     * @warning Asserts if n <= 0 in debug builds
     */
    bool one_in(int n)
    {
        if (n <= 0)
        {
            assert(false && "n must be positive");
            return false;
        }
        return (uniform_dist(n) == 0);
    }

    /**
     * @brief Generates a skewed random number with exponential distribution
     * @param max_log Maximum exponent for the range. Must be non-negative.
     * @return Random number with skewed distribution favoring smaller values
     * 
     * Returns a random number in the range [0, 2^k) where k is randomly chosen
     * from [0, max_log]. This creates a distribution heavily skewed toward smaller
     * values, useful for simulating realistic load patterns.
     * 
     * @warning Asserts if max_log < 0 in debug builds
     */
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
    /**
     * @brief Core XorShift32 algorithm implementation
     * @return Next random 32-bit integer in the sequence
     * 
     * Implements the XorShift32 algorithm using three XOR-shift operations.
     * This is the internal engine that drives all public random number generation.
     */
    uint32_t xorshift32()
    {
        // XorShift32 algorithm
        seed_ ^= (seed_ << 13);
        seed_ ^= (seed_ >> 17);
        seed_ ^= (seed_ << 5);
        return seed_;
    }

    uint32_t seed_; ///< Current state of the random number generator
};

} // namespace Util

#endif // RAND_H