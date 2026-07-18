//! This file implements testing for gbox's ring container types

#include <gtest/gtest.h>

import gbox.core;

/// The ring error enum is reachable through the module and its variants are distinct.
TEST(ringErrorVariants, ringTests) {
    ASSERT_NE(ring::Error::Enqueue, ring::Error::Dequeue);
}

#if 0
// FIXME(bug): OwnedStorage cannot be instantiated. Its only constructor asserts
// `static_assert(list.size() > C, ...)` — the logic is inverted and list.size() is not a
// constant expression — and it derives from memory::MoveOnly, whose default constructor
// is private. Re-enable once OwnedStorage is constructible.
TEST(ringOwnedPushPop, ringTests) {
    ring::RingBuffer<int, 4> buffer = {};

    auto push = buffer.push(1);
    ASSERT_TRUE(push.is_ok());

    auto pop = buffer.pop();
    ASSERT_TRUE(pop.is_ok());
    ASSERT_EQ(1, pop.assume_ok());
}
#endif
