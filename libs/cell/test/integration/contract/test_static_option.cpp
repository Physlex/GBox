//! This file implements contract testing between the static storage type and the option
//! a cell lands it in, covering what a cell may assume about storage once it has landed.

#include <gtest/gtest.h>

#include <probe.hpp>

import gbox.core;
import gbox.funky;

namespace {

/// Identifies the value a test landed in an option
constexpr int LANDED_ID = 6;

}  // namespace

/// Verify storage landing in an option keeps its value, which is the route a cell takes
TEST(staticSurvivesOptionLanding, staticOptionContractTests) {
    Probe::destroyed = 0;

    auto slot = option::Option<memory::Static<Probe>>(option::None());
    slot = option::Some(memory::Static<Probe>(LANDED_ID));

    ASSERT_TRUE(slot.is_some());

    auto ref = memory::StaticRef<Probe>(slot.as_mut());

    ASSERT_EQ(LANDED_ID, ref.get().value);
    ASSERT_EQ(0, Probe::destroyed);
}
