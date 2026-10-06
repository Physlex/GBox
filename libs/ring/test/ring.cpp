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

TEST(ringOwnedIsFullWhenEmpty, ringTests) {
    ring::RingBuffer<int, 1> buffer = {};

    ASSERT_FALSE(buffer.is_full()) << "an empty ring must not report full";
}

TEST(ringOwnedIsFullAfterFillingPush, ringTests) {
    constexpr int VALUE = 1;
    ring::RingBuffer<int, 1> buffer = {};

    ASSERT_TRUE(buffer.push(VALUE).is_ok()) << "push into an empty ring must succeed";

    ASSERT_TRUE(buffer.is_full()) << "a ring at capacity must report full";
}

TEST(ringOwnedIsFullAfterDrainingPop, ringTests) {
    constexpr int VALUE = 1;
    ring::RingBuffer<int, 1> buffer = {};

    ASSERT_TRUE(buffer.push(VALUE).is_ok()) << "push into an empty ring must succeed";
    ASSERT_TRUE(buffer.pop().is_ok()) << "pop from a full ring must succeed";

    ASSERT_FALSE(buffer.is_full()) << "a drained ring must not report full";
}

TEST(ringOwnedIsFullOnlyWhenFilled, ringTests) {
    constexpr int VALUE = 1;
    ring::RingBuffer<int, 2> buffer = {};

    ASSERT_TRUE(buffer.push(VALUE).is_ok()) << "first push must succeed";
    ASSERT_FALSE(buffer.is_full()) << "a ring below capacity must not report full";

    ASSERT_TRUE(buffer.push(VALUE).is_ok()) << "filling push must succeed";
    ASSERT_TRUE(buffer.is_full()) << "a ring at capacity must report full";
}
