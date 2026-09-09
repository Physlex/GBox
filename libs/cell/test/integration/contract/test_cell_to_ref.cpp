//! This file implements contract testing between the static cell and the reference it
//! hands back, covering what a caller may assume about a reference once `init` has
//! returned.

#include <gtest/gtest.h>

#include <probe.hpp>
#include <utility>

import gbox.cell;

namespace {

/// Identifies the value a test settled or wrote through a reference
constexpr int MUTATED_ID = 9;
constexpr int SETTLED_ID = 6;

}  // namespace

/// Verify the reference points into the cell's own storage rather than at a copy
TEST(cellRefPointsIntoCell, cellRefContractTests) {
    auto &slot = cell::StaticCell<Probe>::make();

    auto ref = slot.init(Probe(1)).assume_ok();
    Probe *settled = &ref.get();

    ref->value = MUTATED_ID;

    ASSERT_EQ(settled, &ref.get());
    ASSERT_EQ(MUTATED_ID, ref.get().value);
}

/// Verify the reference survives being moved on, still naming the value the cell settled
TEST(cellRefSurvivesMove, cellRefContractTests) {
    auto &slot = cell::StaticCell<Probe>::make();

    auto ref = slot.init(Probe(4)).assume_ok();
    Probe *settled = &ref.get();

    auto moved = std::move(ref);

    ASSERT_EQ(settled, &moved.get());
    ASSERT_EQ(4, moved.get().value);
}

/// Verify a refused claim hands back no reference, leaving the settled one alone
TEST(cellRefUnaffectedByRefusedClaim, cellRefContractTests) {
    Probe::destroyed = 0;

    auto &slot = cell::StaticCell<Probe>::make();

    auto ref = slot.init(Probe(SETTLED_ID)).assume_ok();
    Probe *settled = &ref.get();

    ASSERT_TRUE(slot.init(Probe(MUTATED_ID)).is_err());

    ASSERT_EQ(settled, &ref.get());
    ASSERT_EQ(SETTLED_ID, ref.get().value);

    // A refused claim never takes the value it was offered, so that value dies with the
    // caller's expression. The value the cell settled is left alone.
    ASSERT_EQ(1, Probe::destroyed);
}
