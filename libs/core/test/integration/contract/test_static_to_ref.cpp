//! This file implements contract testing between the static storage type and the
//! reference formed from it, covering what a reference may assume about the value behind
//! it.

#include <gtest/gtest.h>

#include <probe.hpp>

import gbox.core;

namespace {

/// Identifies the value a test held or wrote through a reference
constexpr int HELD_ID = 4;
constexpr int MUTATED_ID = 8;

}  // namespace

/// Verify a reference observes the value its storage holds rather than a copy of it
TEST(staticRefObservesStorage, staticRefContractTests) {
    auto held = memory::Static<Probe>(HELD_ID);
    auto ref = memory::StaticRef<Probe>(held);

    ref->value = MUTATED_ID;

    ASSERT_EQ(&held.get(), &ref.get());
    ASSERT_EQ(MUTATED_ID, held.get().value);
}
