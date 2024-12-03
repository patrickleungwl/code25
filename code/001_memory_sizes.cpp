#include <gtest/gtest.h>

TEST(MemoryTests, TypeSizeAsserts) {
    ASSERT_EQ(sizeof(char), 1L);
    ASSERT_EQ(sizeof(int), 4L);
    ASSERT_EQ(sizeof(long), 8L);
    ASSERT_EQ(sizeof(double), 8L);
}

