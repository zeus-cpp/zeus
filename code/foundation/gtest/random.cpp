#include <random>
#include <unordered_set>
#include <gtest/gtest.h>
#include <zeus/foundation/core/random.h>

using namespace zeus;

TEST(Random, Rand)
{
    std::random_device         rd;
    std::default_random_engine engine {rd()};
    for (int i = 0; i < 100000; i++)
    {
        {
            std::uniform_int_distribution<unsigned int> uniform(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max());
            auto                                        limitMax = uniform(engine);
            auto                                        limitMin = uniform(engine);
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandUint8();
            EXPECT_LE(RandUint8(limitMax), limitMax);
            auto rand = RandUint8(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }

        {
            std::uniform_int_distribution<uint16_t> uniform;
            auto                                    limitMax = uniform(engine);
            auto                                    limitMin = uniform(engine);
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandUint16();
            EXPECT_LE(RandUint16(limitMax), limitMax);
            auto rand = RandUint16(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }

        {
            std::uniform_int_distribution<uint32_t> uniform;
            auto                                    limitMax = uniform(engine);
            auto                                    limitMin = uniform(engine);
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandUint32();
            EXPECT_LE(RandUint32(limitMax), limitMax);
            auto rand = RandUint32(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }

        {
            std::uniform_int_distribution<uint64_t> uniform;
            auto                                    limitMax = uniform(engine);
            auto                                    limitMin = uniform(engine);
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandUint64();
            EXPECT_LE(RandUint64(limitMax), limitMax);
            auto rand = RandUint64(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }

        {
            std::uniform_real_distribution<float> uniform;
            auto                                  limitMax = uniform(engine);
            auto                                  limitMin = uniform(engine);
            limitMin += 0.1;
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandFloat();
            EXPECT_LE(RandFloat(limitMax), limitMax);
            auto rand = RandFloat(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }

        {
            std::uniform_real_distribution<double> uniform;
            auto                                   limitMax = uniform(engine);
            auto                                   limitMin = uniform(engine);
            if (limitMax < limitMin)
            {
                std::swap(limitMax, limitMin);
            }
            RandDouble();
            EXPECT_LE(RandDouble(limitMax), limitMax);
            auto rand = RandDouble(limitMin, limitMax);
            EXPECT_LE(rand, limitMax);
            EXPECT_GE(rand, limitMin);
        }
        RandBool();
        {
            auto rand = RandLetter();
            EXPECT_TRUE(std::isalpha(rand));
        }
        {
            auto rand = RandNumberLetter();
            EXPECT_TRUE(std::isdigit(rand));
        }
        {
            auto rand = RandLowerLetter();
            EXPECT_TRUE(std::islower(rand));
        }
        {
            auto rand = RandUpperLetter();
            EXPECT_TRUE(std::isupper(rand));
        }
        auto word = RandWord();
        EXPECT_LE(word.size(), 15);
        EXPECT_GE(word.size(), 5);
        RandHex();
        EXPECT_EQ(RandString(100).size(), 100);
        char temp[100];
        RandBytes(temp, 100);
    }
}
