//! This file implements unit testing for the static storage type

#include <gtest/gtest.h>

#include <probe.hpp>
#include <type_traits>
#include <utility>

import gbox.core;

// The storage carries no destructor of its own, whichever branch a type selects, so
// nothing containing it is registered to run at exit.
static_assert(std::is_trivially_destructible_v<memory::Static<Probe>>);
static_assert(std::is_trivially_destructible_v<memory::Static<int>>);

// Storage which is never destroyed may only ever name one value, so it is moved rather
// than copied.
static_assert(std::is_move_constructible_v<memory::Static<Probe>>);
static_assert(!std::is_copy_constructible_v<memory::Static<Probe>>);

namespace {

/// Identifies the value a test placed, so that reading one back names which it was
constexpr int PLACED_ID = 7;
constexpr int EXISTING_ID = 11;
constexpr int ABANDONED_ID = 5;

}  // namespace

/// Construct a `Static` from the arguments of its held type and read the value back
TEST(staticInPlace, staticTests) {
    auto held = memory::Static<Probe>(PLACED_ID);

    ASSERT_EQ(PLACED_ID, held.get().value);
}

/// Construct a `Static` from a value which was built beforehand
TEST(staticFromExistingValue, staticTests) {
    Probe::destroyed = 0;

    auto probe = Probe(EXISTING_ID);
    auto held = memory::Static<Probe>(std::move(probe));

    ASSERT_EQ(EXISTING_ID, held.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}

/// Verify a `Static` leaving scope destroys nothing
TEST(staticNoDestructor, staticTests) {
    Probe::destroyed = 0;

    {
        auto held = memory::Static<Probe>(1);
        ASSERT_EQ(1, held.get().value);
    }

    ASSERT_EQ(0, Probe::destroyed);
}

/// Move a `Static` and verify the value arrives in the destination, destroying nothing
TEST(staticMove, staticTests) {
    Probe::destroyed = 0;

    auto held = memory::Static<Probe>(3);
    auto moved = std::move(held);

    ASSERT_EQ(3, moved.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}

/// Verify the value a `Static` was moved out of is abandoned rather than destroyed
///
/// The source keeps its storage, since nothing is ever destroyed out of a `Static`, but
/// the value in it has given up ownership and no longer counts as live.
TEST(staticMoveAbandonsSource, staticTests) {
    Probe::destroyed = 0;

    {
        auto held = memory::Static<Probe>(ABANDONED_ID);
        auto moved = std::move(held);

        ASSERT_TRUE(moved.get().owner);
        // The source keeps its storage after a move, so reading it back is the assertion.
        // NOLINTNEXTLINE(bugprone-use-after-move,clang-analyzer-cplusplus.Move)
        ASSERT_FALSE(held.get().owner);
    }

    ASSERT_EQ(0, Probe::destroyed);
}

/// Overwrite the value a `Static` holds with the value of another
TEST(staticMoveAssign, staticTests) {
    Probe::destroyed = 0;

    auto held = memory::Static<Probe>(2);
    auto other = memory::Static<Probe>(4);

    held = std::move(other);

    ASSERT_EQ(4, held.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}
