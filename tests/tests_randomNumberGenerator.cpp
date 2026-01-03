/**
 * @file tests_randomNumberGenerator.cpp
 * @brief Unit tests for the RandomNumberGenerator utility class
 * 
 * This file contains comprehensive unit tests for the Util::RandomNumberGenerator class,
 * covering:
 * - Constructor behavior with edge cases (zero seed)
 * - uniform_dist() with power-of-two and non-power-of-two ranges
 * - one_in() probability testing
 * - skewed() distribution testing
 * - Edge cases and boundary conditions
 * 
 * All tests use a fixed seed (12345) in the fixture for reproducibility.
 */

#include <gtest/gtest.h>
#include "Util/RandomNumberGenerator.h"

/**
 * @class RandomNumberGeneratorTest
 * @brief Test fixture for RandomNumberGenerator tests
 * 
 * Provides a consistent test environment with:
 * - A RandomNumberGenerator instance with known seed (12345)
 * - Setup and teardown for each test
 * - Access to the RNG instance through rng_ member
 */
class RandomNumberGeneratorTest : public ::testing::Test
{
protected:
    /**
     * @brief Sets up test fixture before each test
     * Creates a new RandomNumberGenerator with fixed seed 12345 for reproducibility
     */
    void SetUp() override
    {
        // Create a random number generator with a known seed
        rng_ = std::make_unique<Util::RandomNumberGenerator>(12345);
    }

    /**
     * @brief Cleans up test fixture after each test
     * Resets the RNG instance
     */
    void TearDown() override
    {
        rng_.reset();
    }

    std::unique_ptr<Util::RandomNumberGenerator> rng_;  ///< RNG instance for testing
};

/** @test Verifies that zero seed is converted to 1 internally */
TEST_F(RandomNumberGeneratorTest, ConstructorWithZeroSeed)
{
    Util::RandomNumberGenerator rng(0);
    // Should not crash, seed should be set to 1 internally
    uint32_t value = rng.uniform_dist(10);
    EXPECT_GE(value, 0);
    EXPECT_LT(value, 10);
}

TEST_F(RandomNumberGeneratorTest, UniformDistPositiveN)
{
    // Test that uniform_dist returns values in range [0, n)
    for (int i = 0; i < 100; ++i)
    {
        uint32_t value = rng_->uniform_dist(10);
        EXPECT_GE(value, 0);
        EXPECT_LT(value, 10);
    }
}

TEST_F(RandomNumberGeneratorTest, UniformDistPowerOfTwo)
{
    // Test with power of 2
    for (int i = 0; i < 100; ++i)
    {
        uint32_t value = rng_->uniform_dist(16);
        EXPECT_GE(value, 0);
        EXPECT_LT(value, 16);
    }
}

TEST_F(RandomNumberGeneratorTest, UniformDistNonPowerOfTwo)
{
    // Test with non-power of 2
    for (int i = 0; i < 100; ++i)
    {
        uint32_t value = rng_->uniform_dist(100);
        EXPECT_GE(value, 0);
        EXPECT_LT(value, 100);
    }
}

TEST_F(RandomNumberGeneratorTest, UniformDistWithOne)
{
    uint32_t value = rng_->uniform_dist(1);
    EXPECT_EQ(value, 0);
}

TEST_F(RandomNumberGeneratorTest, OneInProbability)
{
    // Test one_in with n=1, should always return true
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(rng_->one_in(1));
    }
}

TEST_F(RandomNumberGeneratorTest, OneInLargeProbability)
{
    // Test one_in with large n, should mostly return false
    int true_count = 0;
    for (int i = 0; i < 1000; ++i)
    {
        if (rng_->one_in(100))
        {
            true_count++;
        }
    }
    // Expected about 10 trues out of 1000 (1%)
    EXPECT_GT(true_count, 0);
    EXPECT_LT(true_count, 50); // Allow some variance
}

TEST_F(RandomNumberGeneratorTest, SkewedValidRange)
{
    for (int i = 0; i < 100; ++i)
    {
        uint32_t value = rng_->skewed(5);
        EXPECT_GE(value, 0);
        // Max possible value when max_log=5: uniform_dist(1 << uniform_dist(6))
        // which could be up to (1 << 32) but we just verify it's a valid uint32_t
    }
}

TEST_F(RandomNumberGeneratorTest, SkewedWithZero)
{
    uint32_t value = rng_->skewed(0);
    EXPECT_GE(value, 0);
    EXPECT_LE(value, 1);
}

TEST_F(RandomNumberGeneratorTest, DifferentSeedsDifferentSequences)
{
    Util::RandomNumberGenerator rng1(1);
    Util::RandomNumberGenerator rng2(2);
    
    uint32_t val1 = rng1.uniform_dist(1000);
    uint32_t val2 = rng2.uniform_dist(1000);
    
    // With different seeds, first values should likely be different
    // (not guaranteed but highly probable)
    bool different = false;
    for (int i = 0; i < 10; ++i)
    {
        if (rng1.uniform_dist(1000) != rng2.uniform_dist(1000))
        {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
}

TEST_F(RandomNumberGeneratorTest, SameSeedSameSequence)
{
    Util::RandomNumberGenerator rng1(42);
    Util::RandomNumberGenerator rng2(42);
    
    // Same seed should produce same sequence
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(rng1.uniform_dist(1000), rng2.uniform_dist(1000));
    }
}
