//! This file implements contract testing between the static storage type and the
//! reference formed from it, covering what a reference may assume about the value behind
//! it.

#include <gtest/gtest.h>

#include <probe.hpp>

import gbox.cell;
import gbox.funky;

namespace {

/// Identifies the value a test held or wrote through a reference
constexpr int HELD_ID = 4;
constexpr int MUTATED_ID = 8;
constexpr int LANDED_ID = 6;

}  // namespace

/// Verify a reference observes the value its storage holds rather than a copy of it
TEST(staticRefObservesStorage, staticRefContractTests) {
    auto held = cell::Static<Probe>(HELD_ID);
    auto ref = cell::StaticRef<Probe>(held);

    ref->value = MUTATED_ID;

    ASSERT_EQ(&held.get(), &ref.get());
    ASSERT_EQ(MUTATED_ID, held.get().value);
}

/// Verify storage landing in an option keeps its value, which is the route a cell takes
TEST(staticSurvivesOptionLanding, staticRefContractTests) {
    Probe::destroyed = 0;

    auto slot = option::Option<cell::Static<Probe>>(option::None());
    slot = option::Some(cell::Static<Probe>(LANDED_ID));

    ASSERT_TRUE(slot.is_some());

    auto ref = cell::StaticRef<Probe>(slot.as_mut());

    ASSERT_EQ(LANDED_ID, ref.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}
