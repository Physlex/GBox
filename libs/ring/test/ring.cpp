//! This file implements testing for gbox's ring container types

#include <gtest/gtest.h>

import gbox.ring;

TEST(ringErrorVariants, ringTests) {
    ASSERT_NE(ring::Error::Enqueue, ring::Error::Dequeue);
}

TEST(ringOwnedPushPop, ringTests) {
    ring::RingBuffer<int, 4> buffer = {};

    auto push = buffer.push(1);
    ASSERT_TRUE(push.is_ok());

    auto pop_res = buffer.pop();
    ASSERT_TRUE(pop_res.is_ok());
    ASSERT_EQ(1, std::move(pop_res).assume_ok());
}

TEST(ringOwnedBounds, ringTests) {
    ring::RingBuffer<int, 2> buffer = {};

    ASSERT_TRUE(buffer.push(1).is_ok());
    ASSERT_TRUE(buffer.push(2).is_ok());
    ASSERT_TRUE(buffer.push(3).is_err());

    ASSERT_TRUE(buffer.pop().is_ok());
    ASSERT_TRUE(buffer.pop().is_ok());
    ASSERT_TRUE(buffer.pop().is_err());
}
