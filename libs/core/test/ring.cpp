//! This file implements testing for gbox's ring container types

#include <gtest/gtest.h>

import gbox.core;

TEST(ringErrorVariants, ringTests) {
    ASSERT_NE(ring::Error::Enqueue, ring::Error::Dequeue);
}

// FIXME: `policy::Ring` declares reader_, writer_ and count_ private, so instantiating
// OwnedStorage's push_impl and pop_impl does not compile. Restore once they are
// reachable.
//
// TEST(ringOwnedPushPop, ringTests) {
//     ring::RingBuffer<int, 4> buffer = {};
//
//     auto push = buffer.push(1);
//     ASSERT_TRUE(push.is_ok());
//
//     auto pop = buffer.pop();
//     ASSERT_TRUE(pop.is_ok());
//     ASSERT_EQ(1, pop.assume_ok());
// }
//
// TEST(ringOwnedBounds, ringTests) {
//     ring::RingBuffer<int, 2> buffer = {};
//
//     ASSERT_TRUE(buffer.push(1).is_ok());
//     ASSERT_TRUE(buffer.push(2).is_ok());
//     ASSERT_TRUE(buffer.push(3).is_err());
//
//     ASSERT_TRUE(buffer.pop().is_ok());
//     ASSERT_TRUE(buffer.pop().is_ok());
//     ASSERT_TRUE(buffer.pop().is_err());
// }
