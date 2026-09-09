//! This file implements unit testing for the static cell type

#include <gtest/gtest.h>

#include <probe.hpp>
#include <type_traits>

import gbox.cell;

// Trivial destructibility survives the whole chain, from the storage through the option
// and into the cell, so a cell registers nothing to run at exit.
static_assert(std::is_trivially_destructible_v<cell::StaticCell<Probe>>);

// A cell is reached through `make` alone, which is what keeps one from being built on a
// stack it would then outlive.
static_assert(!std::is_default_constructible_v<cell::StaticCell<Probe>>);
static_assert(!std::is_move_constructible_v<cell::StaticCell<Probe>>);
static_assert(!std::is_copy_constructible_v<cell::StaticCell<Probe>>);

namespace {

/// Identifies the value a test claimed a cell with, so that reading one back names which
/// it was
constexpr int CLAIMED_ID = 5;

}  // namespace

/// Claim a cell and read the value back through the reference it returns
TEST(cellInitOnce, cellTests) {
    auto &slot = cell::StaticCell<Probe>::make();

    auto claim = slot.init(Probe(CLAIMED_ID));
    ASSERT_TRUE(claim.is_ok());

    auto ref = claim.assume_ok();
    ASSERT_EQ(CLAIMED_ID, ref.get().value);
}

/// Verify a claimed cell refuses every later claim
TEST(cellInitLocked, cellTests) {
    auto &slot = cell::StaticCell<Probe>::make();

    auto first = slot.init(Probe(1));
    ASSERT_TRUE(first.is_ok());

    auto second = slot.init(Probe(2));
    ASSERT_TRUE(second.is_err());
    ASSERT_EQ(cell::Error::Locked, second.assume_err());
}

/// Verify a refused claim leaves the value the cell already holds untouched
TEST(cellLockedKeepsValue, cellTests) {
    auto &slot = cell::StaticCell<Probe>::make();

    auto ref = slot.init(Probe(1)).assume_ok();

    auto refused = slot.init(Probe(2));
    ASSERT_TRUE(refused.is_err());

    ASSERT_EQ(1, ref.get().value);
}

/// Verify claiming a cell destroys nothing it was given
TEST(cellNoDestructor, cellTests) {
    Probe::destroyed = 0;

    auto &slot = cell::StaticCell<Probe>::make();
    auto ref = slot.init(Probe(3)).assume_ok();

    ASSERT_EQ(3, ref.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}

/// Verify each call to `make` names a cell of its own
TEST(cellMakeIsUnique, cellTests) {
    auto &first = cell::StaticCell<Probe>::make();
    auto &second = cell::StaticCell<Probe>::make();

    ASSERT_NE(&first, &second);
    ASSERT_TRUE(first.init(Probe(1)).is_ok());
    ASSERT_TRUE(second.init(Probe(2)).is_ok());
}
